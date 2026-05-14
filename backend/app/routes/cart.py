from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session

from app.database import get_db
from app.schemas.cart import CartAddRequest, CartRemoveRequest, CartUpdateRequest
from app.services import cart_service

router = APIRouter(tags=["cart"])


@router.get("/users/{user_id}/cart")
@router.get("/cart/{user_id}")
def get_cart(user_id: int, db: Session = Depends(get_db)) -> list[dict]:
    return cart_service.get_cart(db, user_id)


@router.post("/users/{user_id}/cart")
def add_to_user_cart(
    user_id: int, payload: CartAddRequest, db: Session = Depends(get_db)
) -> list[dict]:
    return cart_service.add_to_cart(db, user_id, payload.item_id, payload.quantity)


@router.post("/cart/add")
def add_to_cart(payload: CartAddRequest, db: Session = Depends(get_db)) -> list[dict]:
    user_id = payload.user_id or 1
    return cart_service.add_to_cart(db, user_id, payload.item_id, payload.quantity)


@router.put("/users/{user_id}/cart/{item_id}")
def update_cart_item_quantity(
    user_id: int, item_id: int, payload: CartUpdateRequest, db: Session = Depends(get_db)
) -> list[dict]:
    return cart_service.update_cart_quantity(db, user_id, item_id, payload.quantity)


@router.delete("/users/{user_id}/cart/{item_id}")
def remove_from_user_cart(
    user_id: int, item_id: int, db: Session = Depends(get_db)
) -> list[dict]:
    return cart_service.remove_from_cart(db, user_id, item_id)


@router.delete("/cart/remove")
def remove_from_cart(payload: CartRemoveRequest, db: Session = Depends(get_db)) -> list[dict]:
    return cart_service.remove_from_cart(db, payload.user_id, payload.item_id)


@router.post("/users/{user_id}/checkout")
@router.post("/cart/{user_id}/checkout")
def checkout_user_cart(user_id: int, db: Session = Depends(get_db)) -> dict:
    return cart_service.checkout(db, user_id)


@router.post("/cart/checkout")
def checkout_cart(user_id: int = 1, db: Session = Depends(get_db)) -> dict:
    return cart_service.checkout(db, user_id)
