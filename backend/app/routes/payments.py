from __future__ import annotations

import json
import logging

from fastapi import APIRouter, Depends, HTTPException, Query, Request
from sqlalchemy.orm import Session

from app import models
from app.database import get_db
from app.schemas.payment import PayRequest
from app.services import cart_service, payment_service, paymob_service

logger = logging.getLogger(__name__)

router = APIRouter(tags=["payments"])


# ── Existing order-based payment helpers ─────────────────────────────────────

@router.get("/orders/{order_id}/payment")
@router.get("/payments/orders/{order_id}/status")
def get_order_payment_status(order_id: int, db: Session = Depends(get_db)) -> dict:
    status = payment_service.get_payment_status(db, order_id)
    if status["payment_status"] == "paid":
        status["fulfillment"] = cart_service.finalize_paid_order(db, order_id)
    return status


@router.post("/payments/orders/{order_id}/finalize")
def finalize_paid_order(order_id: int, db: Session = Depends(get_db)) -> dict:
    return cart_service.finalize_paid_order(db, order_id)


@router.post("/payments/orders/{order_id}/mock-paid")
def mark_mock_order_paid(order_id: int, db: Session = Depends(get_db)) -> dict:
    payment = payment_service.mark_order_paid(db, order_id)
    fulfillment = cart_service.finalize_paid_order(db, order_id)
    return {
        "success": True,
        "payment": payment,
        "fulfillment": fulfillment,
    }


# ── /pay — 3-step Paymob flow ─────────────────────────────────────────────────

@router.post("/pay")
async def initiate_payment(payload: PayRequest, db: Session = Depends(get_db)) -> dict:
    """
    Runs the full Paymob 3-step flow:
      1. Authenticate  → auth token
      2. Register order → paymob_order_id
      3. Get payment key → iframe URL

    amount must be in EGP; it is converted to cents internally.
    order_ref must match an Order's paymob_reference so the callback can update it.
    """
    result = await paymob_service.create_payment_session(
        amount_egp=payload.amount,
        order_ref=payload.order_ref,
        first_name=payload.first_name,
        last_name=payload.last_name,
        email=payload.email,
        phone=payload.phone,
    )

    # Persist the Paymob order ID against our internal Order/PaymentAttempt
    order = (
        db.query(models.Order)
        .filter(models.Order.paymob_reference == payload.order_ref)
        .first()
    )
    if order:
        attempt = (
            db.query(models.PaymentAttempt)
            .filter(models.PaymentAttempt.order_id == order.id)
            .order_by(models.PaymentAttempt.id.desc())
            .first()
        )
        if attempt:
            existing = json.loads(attempt.raw_response or "{}")
            existing.update(
                {
                    "paymob_order_id": result["paymob_order_id"],
                    "iframe_url": result["iframe_url"],
                    "amount_cents": result["amount_cents"],
                }
            )
            attempt.raw_response = json.dumps(existing)
            db.commit()

    return result


# ── /paymob/callback — webhook from Paymob ───────────────────────────────────

@router.post("/paymob/callback")
async def paymob_callback(
    request: Request,
    hmac_value: str | None = Query(None, alias="hmac"),
    db: Session = Depends(get_db),
) -> dict:
    """
    Receives Paymob's transaction notification webhook.
    Paymob sends: POST /paymob/callback?hmac=<sha512>
    Body: {"type": "TRANSACTION", "obj": { ...transaction fields... }}

    Updates the matching Order to "paid" and triggers fulfillment.
    Always returns HTTP 200 so Paymob does not retry endlessly.
    """
    try:
        body = await request.json()
    except Exception:
        logger.warning("Paymob callback: could not parse JSON body")
        return {"received": True, "error": "invalid json"}

    # Paymob wraps the transaction in "obj" for type=TRANSACTION notifications
    transaction = body.get("obj") or body

    # ── HMAC verification ────────────────────────────────────────────────────
    if hmac_value:
        if not paymob_service.verify_hmac(transaction, hmac_value):
            logger.warning("Paymob callback: HMAC mismatch — possible spoofed request")
            raise HTTPException(status_code=400, detail="HMAC verification failed")
    else:
        logger.info("Paymob callback: no hmac query param — skipping verification (test mode)")

    # ── Extract key fields ────────────────────────────────────────────────────
    success: bool = transaction.get("success", False)
    pending: bool = transaction.get("pending", False)
    transaction_id = transaction.get("id")
    order_data: dict = transaction.get("order") or {}
    merchant_order_id: str = str(order_data.get("merchant_order_id") or "")

    logger.info(
        "Paymob callback: txn_id=%s merchant_order_id=%s success=%s pending=%s",
        transaction_id,
        merchant_order_id,
        success,
        pending,
    )

    if not merchant_order_id:
        logger.warning("Paymob callback: no merchant_order_id in payload")
        return {"received": True, "warning": "no merchant_order_id"}

    # ── Look up our Order ─────────────────────────────────────────────────────
    order = (
        db.query(models.Order)
        .filter(models.Order.paymob_reference == merchant_order_id)
        .first()
    )

    if not order:
        logger.warning("Paymob callback: no Order found for merchant_order_id=%s", merchant_order_id)
        # Return 200 so Paymob does not keep retrying
        return {"received": True, "warning": f"unknown order_ref={merchant_order_id}"}

    # ── Update order status ───────────────────────────────────────────────────
    if success and not pending:
        order.payment_status = "paid"
        attempt = (
            db.query(models.PaymentAttempt)
            .filter(models.PaymentAttempt.order_id == order.id)
            .order_by(models.PaymentAttempt.id.desc())
            .first()
        )
        if attempt:
            attempt.status = "paid"
            if transaction_id:
                attempt.reference = str(transaction_id)
            existing = json.loads(attempt.raw_response or "{}")
            existing["paymob_transaction_id"] = transaction_id
            existing["callback_success"] = True
            attempt.raw_response = json.dumps(existing)
        db.commit()

        # Trigger inventory / interaction fulfillment
        try:
            cart_service.finalize_paid_order(db, order.id)
        except Exception as exc:
            logger.error("Paymob callback: finalize_paid_order failed for order %s: %s", order.id, exc)

        logger.info("Paymob callback: order %s marked paid and fulfilled", order.id)
        return {"received": True, "order_id": order.id, "status": "paid"}

    # Pending or failed — just log, do not change status
    logger.info(
        "Paymob callback: order %s not paid yet (success=%s pending=%s)", order.id, success, pending
    )
    return {"received": True, "order_id": order.id, "status": "pending" if pending else "failed"}
