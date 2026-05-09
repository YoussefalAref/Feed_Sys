from fastapi import HTTPException
from sqlalchemy.orm import Session

from app import models
from app.services import cpp_core
from app.services import interaction_service
from app.services import payment_service
from app.services.serialization import cart_item_to_dict


def get_cart(db: Session, user_id: int) -> list[dict]:
    core = cpp_core.load_core()
    if core and hasattr(core, "get_cart"):
        try:
            result = core.get_cart(user_id)
            if isinstance(result, list):
                return result
        except Exception:
            pass

    rows = (
        db.query(models.CartItem)
        .filter(models.CartItem.user_id == user_id)
        .order_by(models.CartItem.id)
        .all()
    )
    return [cart_item_to_dict(row) for row in rows]


def add_to_cart(db: Session, user_id: int, item_id: int, quantity: int = 1) -> list[dict]:
    if quantity < 1:
        raise HTTPException(status_code=400, detail="Quantity must be at least 1")

    user = db.get(models.User, user_id)
    product = db.get(models.Item, item_id)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    if not product:
        raise HTTPException(status_code=404, detail="Product not found")

    core = cpp_core.load_core()
    if core and hasattr(core, "add_to_cart"):
        try:
            result = core.add_to_cart(user_id, item_id, quantity)
            if isinstance(result, list):
                return result
            if isinstance(result, bool) and result:
                return get_cart(db, user_id)
        except Exception:
            pass

    existing = (
        db.query(models.CartItem)
        .filter(models.CartItem.user_id == user_id, models.CartItem.item_id == item_id)
        .first()
    )
    if existing:
        existing.quantity += quantity
    else:
        db.add(models.CartItem(user_id=user_id, item_id=item_id, quantity=quantity))
    db.commit()

    interaction_service.record_interaction(db, user_id, item_id, "cart")
    return get_cart(db, user_id)


def remove_from_cart(db: Session, user_id: int, item_id: int) -> list[dict]:
    core = cpp_core.load_core()
    if core and hasattr(core, "remove_from_cart"):
        try:
            result = core.remove_from_cart(user_id, item_id)
            if isinstance(result, list):
                return result
            if isinstance(result, bool):
                return get_cart(db, user_id)
        except Exception:
            pass

    row = (
        db.query(models.CartItem)
        .filter(models.CartItem.user_id == user_id, models.CartItem.item_id == item_id)
        .first()
    )
    if row:
        db.delete(row)
        db.commit()
    return get_cart(db, user_id)


def checkout(db: Session, user_id: int) -> dict:
    user = db.get(models.User, user_id)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")

    current_cart = (
        db.query(models.CartItem)
        .filter(models.CartItem.user_id == user_id)
        .order_by(models.CartItem.id)
        .all()
    )
    if not current_cart:
        raise HTTPException(status_code=400, detail="Cart is empty")

    core = cpp_core.load_core()
    if core and hasattr(core, "checkout"):
        try:
            result = core.checkout(user_id)
            if result is not None:
                return result
        except Exception:
            pass

    total = sum(row.item.price * row.quantity for row in current_cart)
    order = models.Order(user_id=user_id, total=total, payment_status="pending")
    db.add(order)
    db.commit()
    db.refresh(order)

    for entry in current_cart:
        db.add(
            models.OrderItem(
                order_id=order.id,
                item_id=entry.item_id,
                name=entry.item.name,
                price=entry.item.price,
                quantity=entry.quantity,
            )
        )

    db.commit()
    payment = payment_service.start_paymob_payment(db, order)

    return {
        "success": True,
        "message": "Order created. Complete payment with Paymob before purchase is finalized.",
        "purchased": 0,
        "order_id": order.id,
        "payment_status": payment["payment_status"],
        "payment_reference": payment["payment_reference"],
        "paymob_reference": payment["paymob_reference"],
        "fawry_reference": payment["fawry_reference"],
        "payment_attempt": payment["payment_attempt"],
    }


def finalize_paid_order(db: Session, order_id: int) -> dict:
    order = db.get(models.Order, order_id)
    if not order:
        raise HTTPException(status_code=404, detail="Order not found")
    if order.payment_status != "paid":
        raise HTTPException(status_code=400, detail="Order is not paid yet")
    if order.fulfillment_status == "fulfilled":
        return {
            "success": True,
            "purchased": 0,
            "order_id": order.id,
            "payment_status": order.payment_status,
            "fulfillment_status": order.fulfillment_status,
        }

    order_items = (
        db.query(models.OrderItem)
        .filter(models.OrderItem.order_id == order_id)
        .order_by(models.OrderItem.id)
        .all()
    )
    for entry in order_items:
        interaction_service.record_interaction(db, order.user_id, entry.item_id, "purchase")
        cart_row = (
            db.query(models.CartItem)
            .filter(
                models.CartItem.user_id == order.user_id,
                models.CartItem.item_id == entry.item_id,
            )
            .first()
        )
        if cart_row:
            db.delete(cart_row)

    order.fulfillment_status = "fulfilled"
    db.commit()
    return {
        "success": True,
        "purchased": len(order_items),
        "order_id": order.id,
        "payment_status": order.payment_status,
        "fulfillment_status": order.fulfillment_status,
    }
