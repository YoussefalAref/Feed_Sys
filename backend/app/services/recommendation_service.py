"""
Recommendation Service - Thin Python wrapper over C++ recommendation functions.

According to Backend API Contract:
- C++ get_recommendations(user_id, limit) -> std::vector<RecommendationDTO>
- C++ get_related_products(item_id, limit) -> std::vector<RelatedProductDTO>  
- C++ get_trending(limit) -> std::vector<ProductDTO>

This service:
1. Calls C++ functions through pybind11
2. Falls back to SQLAlchemy if C++ module not available
3. Converts C++ DTOs to Python dicts for HTTP responses
"""

from sqlalchemy.orm import Session

from app import models
from app.services import cpp_core
from app.services.serialization import product_to_dict


def _merge_core_products(
    core_products: list[dict],
    products_by_id: dict[int, dict],
    score_field: str | None = None,
) -> list[dict]:
    """Helper to merge C++ product results with database product data."""
    merged = []
    for product in core_products:
        product_id = product.get("id")
        original = products_by_id.get(product_id)
        if not original:
            merged.append(product)
            continue

        combined = {**original, **product}
        if score_field:
            combined[score_field] = product.get("popularity_score", product.get(score_field))
            combined["popularity_score"] = original["popularity_score"]

        merged.append(combined)
    return merged


def get_recommendations(db: Session, user_id: int, limit: int = 4) -> list[dict]:
    """
    Get personalized recommendations for a user.
    
    Contract: C++ get_recommendations(user_id, limit) -> std::vector<RecommendationDTO>
    
    For new/unknown users, falls back to top products.
    For known users, combines:
    - Item popularity score
    - Graph similarity to recently interacted products
    - Category/user interest boost
    - Optional recency weight
    """
    user = db.get(models.User, user_id)
    products = (
        db.query(models.Item)
        .order_by(models.Item.popularity_score.desc(), models.Item.id.asc())
        .all()
    )
    product_dicts = [product_to_dict(product) for product in products]

    core = cpp_core.load_core()
    
    if core and hasattr(core, 'get_recommendations') and user:
        try:
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
            if isinstance(core_recommendations, list):
                return _merge_core_products(
                    core_recommendations,
                    {product["id"]: product for product in product_dicts},
                    score_field="recommendation_score",
                )
        except Exception:
            # Fall through to database-backed implementation
            pass

    # Fallback: Return top products by category or overall popularity
    if user:
        preferred = user.category
        recommended = [
            *[item for item in products if item.category == preferred],
            *[item for item in products if item.category != preferred],
        ][:limit]
    else:
        recommended = products[:limit]
    
    return [product_to_dict(product) for product in recommended]


def get_related_products(db: Session, item_id: int, limit: int = 3) -> list[dict]:
    """
    Get products related to a given item.
    
    Contract: C++ get_related_products(item_id, limit) -> std::vector<RelatedProductDTO>
    
    Uses graph neighbors first, sorted by similarity weight then popularity.
    Falls back to same-category products if graph has no neighbors.
    """
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
    
    if core and hasattr(core, 'get_related_products'):
        try:
            core_related = core.get_related_products(product_dicts, item_id, limit)
            if isinstance(core_related, list):
                return _merge_core_products(
                    core_related,
                    {item["id"]: item for item in product_dicts},
                    score_field="similarity_score",
                )
        except Exception:
            # Fall through to database-backed implementation
            pass

    # Fallback: Same-category products
    related = (
        db.query(models.Item)
        .filter(models.Item.id != item_id, models.Item.category == product.category)
        .order_by(models.Item.popularity_score.desc(), models.Item.id.asc())
        .limit(limit)
        .all()
    )
    return [product_to_dict(item) for item in related]


def get_trending(db: Session, limit: int = 8) -> list[dict]:
    """
    Get trending products.
    
    Contract: C++ get_trending(limit) -> std::vector<ProductDTO>
    
    Uses max heap or ranking structure.
    Returns highest popularity products first.
    """
    products = (
        db.query(models.Item)
        .order_by(models.Item.popularity_score.desc(), models.Item.id.asc())
        .all()
    )
    product_dicts = [product_to_dict(product) for product in products]

    core = cpp_core.load_core()
    
    # If the C++ binding exposes a rank function that accepts a product list,
    # call it using the locally assembled `product_dicts`.
    if core and hasattr(core, 'get_trending_via_rank'):
        try:
            core_ranked = core.get_trending_via_rank(product_dicts, limit)
            if isinstance(core_ranked, list):
                return _merge_core_products(
                    core_ranked,
                    {product["id"]: product for product in product_dicts},
                )
        except Exception:
            # Fall through to database-backed implementation
            pass

    # Fallback: Top products by popularity
    return product_dicts[:limit]
