import logging

from sqlalchemy.orm import Session

from app import models
from app.services.serialization import product_to_dict
from app.services import cpp_core


logger = logging.getLogger(__name__)


def _merge_core_products(
    core_products: list[dict],
    products_by_id: dict[int, dict],
    score_field: str | None = None,
) -> list[dict]:
    merged = []
    for product in core_products:
        product_id = product.get("id")
        original = products_by_id.get(product_id)
        if not original:
            merged.append(product)
            continue

        combined = {**original, **product}
        if score_field:
            combined[score_field] = product.get("popularity_score")
            combined["popularity_score"] = original["popularity_score"]

        merged.append(combined)
    return merged


def get_recommendations(db: Session, user_id: int, limit: int = 4) -> list[dict]:
    user = db.get(models.User, user_id)
    products = (
        db.query(models.Item)
        .order_by(models.Item.popularity_score.desc(), models.Item.id.asc())
        .all()
    )
    product_dicts = [product_to_dict(product) for product in products]

    core = cpp_core.load_core()
    if core and user:
        try:
            logger.info("Using C++ core for recommendations for user_id=%s", user_id)
            interactions = (
                db.query(models.Interaction)
                .filter(models.Interaction.user_id == user_id)
                .order_by(models.Interaction.timestamp.asc(), models.Interaction.id.asc())
                .all()
            )
            core_recommendations = core.get_recommendations(
                product_dicts,
                [
                    {
                        "id": interaction.id,
                        "user_id": interaction.user_id,
                        "item_id": interaction.item_id,
                        "type": interaction.type,
                        "timestamp": interaction.timestamp.isoformat(),
                    }
                    for interaction in interactions
                ],
                {
                    "id": user.id,
                    "email": user.email,
                    "category": user.category,
                },
                limit,
            )
            return _merge_core_products(
                core_recommendations,
                {product["id"]: product for product in product_dicts},
                score_field="recommendation_score",
            )
        except Exception:
            logger.exception("C++ core recommendation path failed; using Python fallback for user_id=%s", user_id)
            pass
    else:
        logger.info(
            "C++ core not used for recommendations; core_loaded=%s user_present=%s user_id=%s",
            bool(core),
            bool(user),
            user_id,
        )

    preferred = user.category if user else None
    recommended = [
        *[item for item in products if item.category == preferred],
        *[item for item in products if item.category != preferred],
    ][:limit]
    return [product_to_dict(product) for product in recommended]


def get_related_products(db: Session, item_id: int, limit: int = 3) -> list[dict]:
    product = db.get(models.Item, item_id)
    if not product:
        return []

    products = (
        db.query(models.Item)
        .order_by(models.Item.popularity_score.desc(), models.Item.id.asc())
        .all()
    )
    product_dicts = [product_to_dict(item) for item in products]

    core = cpp_core.load_core()
    if core:
        try:
            logger.info("Using C++ core for related products for item_id=%s", item_id)
            core_related = core.get_related_products(product_dicts, item_id, limit)
            return _merge_core_products(
                core_related,
                {item["id"]: item for item in product_dicts},
            )
        except Exception:
            logger.exception("C++ core related-products path failed; using Python fallback for item_id=%s", item_id)
            pass

    logger.info("Using Python fallback for related products for item_id=%s", item_id)

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
        .all()
    )
    product_dicts = [product_to_dict(product) for product in products]

    core = cpp_core.load_core()
    if core:
        try:
            logger.info("Using C++ core for trending products limit=%s", limit)
            core_ranked = core.get_trending(product_dicts, limit)
            return _merge_core_products(
                core_ranked,
                {product["id"]: product for product in product_dicts},
            )
        except Exception:
            logger.exception("C++ core trending path failed; using Python fallback limit=%s", limit)
            pass

    logger.info("Using Python fallback for trending products limit=%s", limit)
    return product_dicts[:limit]
