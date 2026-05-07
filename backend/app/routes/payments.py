from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session

from app.database import get_db
from app.services import cart_service, payment_service

router = APIRouter(tags=["payments"])


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
