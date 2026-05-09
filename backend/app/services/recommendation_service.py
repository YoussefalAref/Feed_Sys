from sqlalchemy.orm import Session

from app import models
from app.services.serialization import product_to_dict


def get_recommendations(db: Session, user_id: int, limit: int = 4) -> list[dict]:
    user = db.get(models.User, user_id)
    preferred = user.category if user else None
    products = (
        db.query(models.Item)
        .order_by(models.Item.popularity_score.desc(), models.Item.id.asc())
        .all()
    )
    recommended = [
        *[item for item in products if item.category == preferred],
        *[item for item in products if item.category != preferred],
    ][:limit]
    return [product_to_dict(product) for product in recommended]


def get_related_products(db: Session, item_id: int, limit: int = 3) -> list[dict]:
    product = db.get(models.Item, item_id)
    if not product:
        return []

    related = (
        db.query(models.Item)
        .filter(models.Item.id != item_id, models.Item.category == product.category)
        .order_by(models.Item.popularity_score.desc(), models.Item.id.asc())
        .limit(limit)
        .all()
    )
    return [product_to_dict(item) for item in related]


def get_trending(db: Session, limit: int = 8) -> list[dict]:
    products = (
        db.query(models.Item)
        .order_by(models.Item.popularity_score.desc(), models.Item.id.asc())
        .limit(limit)
        .all()
    )
    return [product_to_dict(product) for product in products]
