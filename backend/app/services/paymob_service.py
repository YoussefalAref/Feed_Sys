"""
Paymob legacy Accept API — 3-step checkout flow.

Step 1  POST /api/auth/tokens              → auth_token
Step 2  POST /api/ecommerce/orders         → paymob_order_id
Step 3  POST /api/acceptance/payment_keys  → payment_key

iframe URL: https://accept.paymob.com/api/acceptance/iframes/{IFRAME_ID}?payment_token={payment_key}
"""
from __future__ import annotations

import hashlib
import hmac as hmac_lib
import logging
import os

import httpx
from fastapi import HTTPException

logger = logging.getLogger(__name__)

_PAYMOB_BASE = "https://accept.paymob.com/api"
_IFRAME_BASE = "https://accept.paymob.com/api/acceptance/iframes"


def _cfg(name: str, default: str = "") -> str:
    return os.getenv(name, default).strip()


# ── Step 1 ───────────────────────────────────────────────────────────────────

async def _authenticate(client: httpx.AsyncClient) -> str:
    api_key = _cfg("PAYMOB_API_KEY")
    if not api_key:
        raise HTTPException(status_code=500, detail="PAYMOB_API_KEY is not configured")

    resp = await client.post(
        f"{_PAYMOB_BASE}/auth/tokens",
        json={"api_key": api_key},
    )
    resp.raise_for_status()
    token = resp.json().get("token")
    if not token:
        raise HTTPException(status_code=502, detail=f"Paymob auth returned no token: {resp.text}")
    return token


# ── Step 2 ───────────────────────────────────────────────────────────────────

async def _register_order(
    client: httpx.AsyncClient,
    auth_token: str,
    amount_cents: int,
    order_ref: str,
) -> int:
    resp = await client.post(
        f"{_PAYMOB_BASE}/ecommerce/orders",
        json={
            "auth_token": auth_token,
            "delivery_needed": False,
            "amount_cents": amount_cents,
            "currency": _cfg("PAYMOB_CURRENCY", "EGP"),
            "merchant_order_id": order_ref,
            "items": [],
        },
    )
    resp.raise_for_status()
    paymob_order_id = resp.json().get("id")
    if not paymob_order_id:
        raise HTTPException(
            status_code=502,
            detail=f"Paymob order registration returned no id: {resp.text}",
        )
    return paymob_order_id


# ── Step 3 ───────────────────────────────────────────────────────────────────

async def _get_payment_key(
    client: httpx.AsyncClient,
    auth_token: str,
    amount_cents: int,
    paymob_order_id: int,
    billing: dict,
) -> str:
    integration_id = _cfg("PAYMOB_INTEGRATION_ID")
    if not integration_id:
        raise HTTPException(status_code=500, detail="PAYMOB_INTEGRATION_ID is not configured")

    resp = await client.post(
        f"{_PAYMOB_BASE}/acceptance/payment_keys",
        json={
            "auth_token": auth_token,
            "amount_cents": amount_cents,
            "expiration": 3600,
            "order_id": paymob_order_id,
            "billing_data": billing,
            "currency": _cfg("PAYMOB_CURRENCY", "EGP"),
            "integration_id": int(integration_id),
            "lock_order_when_paid": False,
        },
    )
    resp.raise_for_status()
    payment_key = resp.json().get("token")
    if not payment_key:
        raise HTTPException(
            status_code=502,
            detail=f"Paymob payment key returned no token: {resp.text}",
        )
    return payment_key


# ── Public entry-point ───────────────────────────────────────────────────────

async def create_payment_session(
    amount_egp: float,
    order_ref: str,
    first_name: str = "Customer",
    last_name: str = "Customer",
    email: str = "customer@biteapple.test",
    phone: str = "+201234567890",
) -> dict:
    """
    Full 3-step Paymob flow.
    amount_egp is in Egyptian Pounds; converted to cents internally.
    Returns a dict with iframe_url, payment_token, paymob_order_id, amount_cents.
    """
    iframe_id = _cfg("PAYMOB_IFRAME_ID")
    if not iframe_id:
        raise HTTPException(status_code=500, detail="PAYMOB_IFRAME_ID is not configured")

    amount_cents = int(round(amount_egp * 100))

    billing = {
        "apartment": "NA",
        "email": email,
        "floor": "NA",
        "first_name": first_name,
        "street": "NA",
        "building": "NA",
        "phone_number": phone,
        "shipping_method": "NA",
        "postal_code": "NA",
        "city": "Cairo",
        "country": "EG",
        "last_name": last_name,
        "state": "NA",
    }

    try:
        async with httpx.AsyncClient(timeout=30.0) as client:
            auth_token = await _authenticate(client)
            logger.info("Paymob: authenticated for order_ref=%s", order_ref)

            paymob_order_id = await _register_order(client, auth_token, amount_cents, order_ref)
            logger.info("Paymob: registered order paymob_id=%s", paymob_order_id)

            payment_key = await _get_payment_key(
                client, auth_token, amount_cents, paymob_order_id, billing
            )
            logger.info("Paymob: got payment key for order_ref=%s", order_ref)

    except HTTPException:
        raise
    except httpx.HTTPStatusError as exc:
        logger.error(
            "Paymob HTTP %s error: %s", exc.response.status_code, exc.response.text
        )
        raise HTTPException(
            status_code=502,
            detail=f"Paymob returned HTTP {exc.response.status_code}: {exc.response.text}",
        )
    except httpx.RequestError as exc:
        logger.error("Paymob network error: %s", exc)
        raise HTTPException(status_code=502, detail="Could not reach Paymob — check network.")

    iframe_url = f"{_IFRAME_BASE}/{iframe_id}?payment_token={payment_key}"

    return {
        "iframe_url": iframe_url,
        "payment_token": payment_key,
        "paymob_order_id": paymob_order_id,
        "amount_cents": amount_cents,
        "order_ref": order_ref,
    }


# ── HMAC verification ────────────────────────────────────────────────────────

def verify_hmac(transaction_obj: dict, received_hmac: str) -> bool:
    """
    Verify Paymob HMAC-SHA512 webhook signature.
    If PAYMOB_HMAC_SECRET is not configured, verification is skipped (returns True).
    """
    secret = _cfg("PAYMOB_HMAC_SECRET")
    if not secret:
        logger.warning("PAYMOB_HMAC_SECRET not set — skipping HMAC verification (test mode)")
        return True

    # Paymob concatenates these fields in this exact order
    fields = [
        str(transaction_obj.get("amount_cents", "")),
        str(transaction_obj.get("created_at", "")),
        str(transaction_obj.get("currency", "")),
        str(transaction_obj.get("error_occured", "")),
        str(transaction_obj.get("has_parent_transaction", "")),
        str(transaction_obj.get("id", "")),
        str(transaction_obj.get("integration_id", "")),
        str(transaction_obj.get("is_3d_secure", "")),
        str(transaction_obj.get("is_auth", "")),
        str(transaction_obj.get("is_capture", "")),
        str(transaction_obj.get("is_refunded", "")),
        str(transaction_obj.get("is_standalone_payment", "")),
        str(transaction_obj.get("is_voided", "")),
        str(transaction_obj.get("order", {}).get("id", "")),
        str(transaction_obj.get("owner", "")),
        str(transaction_obj.get("pending", "")),
        str(transaction_obj.get("source_data", {}).get("pan", "")),
        str(transaction_obj.get("source_data", {}).get("sub_type", "")),
        str(transaction_obj.get("source_data", {}).get("type", "")),
        str(transaction_obj.get("success", "")),
    ]

    concatenated = "".join(fields)
    computed = hmac_lib.new(
        secret.encode("utf-8"),
        concatenated.encode("utf-8"),
        hashlib.sha512,
    ).hexdigest()

    return hmac_lib.compare_digest(computed, received_hmac.lower())
