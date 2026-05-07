from __future__ import annotations

import json
import os
from datetime import UTC, datetime

from fastapi import HTTPException
from sqlalchemy.orm import Session

from app import models

PAYMENT_PROVIDER = "paymob"
DEFAULT_CHECKOUT_BASE_URL = "https://accept.paymob.com/standalone"


def _env(name: str, default: str = "") -> str:
    return os.getenv(name, default).strip()


def _money(value: float) -> str:
    return f"{value:.2f}"


def _is_configured() -> bool:
    return bool(_env("PAYMOB_API_KEY") and _env("PAYMOB_INTEGRATION_ID"))


def _mock_mode() -> bool:
    configured = _env("PAYMOB_MOCK_MODE", "").lower()
    if configured in {"1", "true", "yes", "on"}:
        return True
    if configured in {"0", "false", "no", "off"}:
        return False
    return not _is_configured()


def _payment_reference(order_id: int) -> str:
    return f"PM-{order_id}-{int(datetime.now(UTC).timestamp())}"


def _checkout_payload(order: models.Order, reference: str) -> dict:
    return {
        "provider": PAYMENT_PROVIDER,
        "order_id": order.id,
        "reference": reference,
        "amount": _money(order.total),
        "currency": _env("PAYMOB_CURRENCY", "EGP"),
        "checkout_url": _env("PAYMOB_CHECKOUT_URL", DEFAULT_CHECKOUT_BASE_URL),
        "public_key": _env("PAYMOB_PUBLIC_KEY"),
        "integration_id": _env("PAYMOB_INTEGRATION_ID"),
    }


def _payment_attempt_to_dict(attempt: models.PaymentAttempt | None) -> dict | None:
    if not attempt:
        return None
    return {
        "id": attempt.id,
        "provider": attempt.provider,
        "reference": attempt.reference,
        "status": attempt.status,
        "created_at": attempt.created_at.isoformat(),
    }


def _order_to_dict(order: models.Order, attempt: models.PaymentAttempt | None = None) -> dict:
    payment_reference = order.paymob_reference or order.fawry_reference
    return {
        "order_id": order.id,
        "user_id": order.user_id,
        "total": order.total,
        "payment_status": order.payment_status,
        "payment_reference": payment_reference,
        "paymob_reference": order.paymob_reference,
        "fawry_reference": order.fawry_reference,
        "fulfillment_status": order.fulfillment_status,
        "created_at": order.created_at.isoformat(),
        "payment_attempt": _payment_attempt_to_dict(attempt),
    }


def _latest_attempt(db: Session, order_id: int) -> models.PaymentAttempt | None:
    return (
        db.query(models.PaymentAttempt)
        .filter(models.PaymentAttempt.order_id == order_id)
        .order_by(models.PaymentAttempt.id.desc())
        .first()
    )


def start_paymob_payment(db: Session, order: models.Order) -> dict:
    user = db.get(models.User, order.user_id)
    if not user:
        raise HTTPException(status_code=404, detail="Order user not found")

    if _mock_mode():
        reference = _payment_reference(order.id)
        response = {
            "type": "MockPaymobIntentionResponse",
            "provider": PAYMENT_PROVIDER,
            "reference": reference,
            "payment_status": "pending",
            "checkout_url": f"{_env('PAYMOB_CHECKOUT_URL', DEFAULT_CHECKOUT_BASE_URL).rstrip('/')}/{reference}",
            "client_secret": f"mock_client_secret_{reference}",
            "status_code": 200,
            "status_description": "Mock Paymob checkout created",
        }
    else:
        reference = _payment_reference(order.id)
        response = _checkout_payload(order, reference)

    order.paymob_reference = reference
    order.fawry_reference = reference
    order.payment_status = "pending"
    attempt = models.PaymentAttempt(
        order_id=order.id,
        provider=PAYMENT_PROVIDER,
        reference=reference,
        status="pending",
        raw_response=json.dumps(response),
    )
    db.add(attempt)
    db.commit()
    db.refresh(order)
    db.refresh(attempt)

    result = _order_to_dict(order, attempt)
    result["raw_status"] = response
    return result


def get_payment_status(db: Session, order_id: int) -> dict:
    order = db.get(models.Order, order_id)
    if not order:
        raise HTTPException(status_code=404, detail="Order not found")

    attempt = _latest_attempt(db, order_id)
    if not attempt:
        return _order_to_dict(order, None)

    response = json.loads(attempt.raw_response or "{}")
    status = attempt.status
    if not _mock_mode():
        response.setdefault("provider", PAYMENT_PROVIDER)
        response.setdefault("payment_status", status)

    result = _order_to_dict(order, attempt)
    result["raw_status"] = response
    return result


def mark_order_paid(db: Session, order_id: int) -> dict:
    order = db.get(models.Order, order_id)
    if not order:
        raise HTTPException(status_code=404, detail="Order not found")

    order.payment_status = "paid"
    attempt = _latest_attempt(db, order_id)
    if attempt:
        attempt.status = "paid"
    db.commit()
    db.refresh(order)
    if attempt:
        db.refresh(attempt)
    return _order_to_dict(order, attempt)


start_fawry_payment = start_paymob_payment
