from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session

from app.database import get_db
from app.schemas.product import ProductInput
from app.services import product_service

router = APIRouter(tags=["products"])


@router.get("/products")
def list_products(category: str | None = None, db: Session = Depends(get_db)) -> list[dict]:
    return product_service.list_products(db, category)


@router.get("/products/category/{category}")
def list_products_by_category(category: str, db: Session = Depends(get_db)) -> list[dict]:
    return product_service.list_products(db, category)


@router.get("/products/{item_id}")
def get_product(item_id: int, db: Session = Depends(get_db)) -> dict:
    return product_service.get_product(db, item_id)


@router.post("/products")
@router.post("/manager/products")
def create_product(payload: ProductInput, db: Session = Depends(get_db)) -> dict:
    return product_service.create_product(db, payload.model_dump())


@router.put("/products/{item_id}")
@router.put("/manager/products/{item_id}")
def update_product(item_id: int, payload: ProductInput, db: Session = Depends(get_db)) -> dict:
    return product_service.update_product(db, item_id, payload.model_dump())


@router.delete("/products/{item_id}")
@router.delete("/manager/products/{item_id}")
def delete_product(item_id: int, db: Session = Depends(get_db)) -> dict:
    return product_service.delete_product(db, item_id)
