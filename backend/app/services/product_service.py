from fastapi import HTTPException
from sqlalchemy.orm import Session

from app import models
from app.services.serialization import product_to_dict


def list_products(db: Session, category: str | None = None) -> list[dict]:
    query = db.query(models.Item)
    if category:
        query = query.filter(models.Item.category.ilike(category))
    return [product_to_dict(product) for product in query.order_by(models.Item.id).all()]


def get_product(db: Session, item_id: int) -> dict:
    product = db.get(models.Item, item_id)
    if not product:
        raise HTTPException(status_code=404, detail="Product not found")
    return product_to_dict(product)


def create_product(db: Session, payload: dict) -> dict:
    product = models.Item(
        name=payload["name"],
        price=float(payload["price"]),
        category=payload["category"],
        stock=int(payload.get("stock", 0)),
        image=payload.get("image", ""),
        description=payload.get("description", ""),
        popularity_score=float(payload.get("popularity_score", 40)),
    )
    db.add(product)
    db.commit()
    db.refresh(product)
    return product_to_dict(product)


def update_product(db: Session, item_id: int, payload: dict) -> dict:
    product = db.get(models.Item, item_id)
    if not product:
        raise HTTPException(status_code=404, detail="Product not found")

    product.name = payload["name"]
    product.price = float(payload["price"])
    product.category = payload["category"]
    product.stock = int(payload.get("stock", product.stock))
    product.image = payload.get("image", product.image)
    product.description = payload.get("description", product.description)
    product.popularity_score = float(payload.get("popularity_score", product.popularity_score))
    db.commit()
    db.refresh(product)
    return product_to_dict(product)


def delete_product(db: Session, item_id: int) -> dict:
    product = db.get(models.Item, item_id)
    if not product:
        raise HTTPException(status_code=404, detail="Product not found")

    db.query(models.CartItem).filter(models.CartItem.item_id == item_id).delete()
    db.query(models.Interaction).filter(models.Interaction.item_id == item_id).delete()
    db.delete(product)
    db.commit()
    return {"success": True}
