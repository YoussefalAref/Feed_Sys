"""
Interaction Service - Thin Python wrapper over C++ interaction functions.

According to Backend API Contract:
- C++ record_interaction(user_id, item_id, interaction_type) -> InteractionDTO
- C++ get_recent_interactions(limit) -> std::vector<InteractionDTO>

This service:
1. Calls C++ functions through pybind11
2. Falls back to SQLAlchemy if C++ module not available
3. Converts C++ DTOs to Python dicts for HTTP responses
"""

from datetime import UTC, datetime

from fastapi import HTTPException
from sqlalchemy.orm import Session

from app import models
from app.services import cpp_core
from app.services.serialization import interaction_to_dict

ALLOWED_TYPES = {"view", "click", "cart", "purchase"}


def record_interaction(db: Session, user_id: int, item_id: int, interaction_type: str) -> dict:
    """
    Record a user interaction with an item.
    
    Contract: C++ record_interaction(user_id, item_id, interaction_type) -> InteractionDTO
    
    Allowed types: "view", "click", "cart", "purchase"
    (Mapped to C++ enum: VIEW, CLICK, ADD_TO_CART, PURCHASE)
    """
    if interaction_type not in ALLOWED_TYPES:
        raise HTTPException(status_code=400, detail="Unsupported interaction type")

    user = db.get(models.User, user_id)
    product = db.get(models.Item, item_id)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    if not product:
        raise HTTPException(status_code=404, detail="Product not found")

    core = cpp_core.load_core()
    
    if core and hasattr(core, 'record_interaction'):
        try:
            result = core.record_interaction(user_id, item_id, interaction_type)
            if result is not None:
                return result
        except Exception:
            # Fall through to database-backed implementation
            pass
    
    # Fallback: Database-backed interaction recording
    interaction = models.Interaction(
        user_id=user_id,
        item_id=item_id,
        type=interaction_type,
        timestamp=datetime.now(UTC).replace(tzinfo=None),
    )
    db.add(interaction)
    db.commit()
    db.refresh(interaction)
    return interaction_to_dict(interaction)


def get_recent_interactions(db: Session, limit: int = 8) -> list[dict]:
    """
    Get recent interactions.
    
    Contract: C++ get_recent_interactions(limit) -> std::vector<InteractionDTO>
    """
    core = cpp_core.load_core()
    
    if core and hasattr(core, 'get_recent_interactions'):
        try:
            # Fetch all interactions from DB and pass to C++ for filtering
            all_interactions = (
                db.query(models.Interaction)
                .order_by(models.Interaction.timestamp.asc(), models.Interaction.id.asc())
                .all()
            )
            interactions_dict = [interaction_to_dict(i) for i in all_interactions]
            result = core.get_recent_interactions(interactions_dict, limit)
            if isinstance(result, list):
                return result
        except Exception:
            # Fall through to database-backed implementation
            pass

    # Fallback: Database-backed recent interactions
    interactions = (
        db.query(models.Interaction)
        .order_by(models.Interaction.timestamp.desc(), models.Interaction.id.desc())
        .limit(limit)
        .all()
    )
    return [interaction_to_dict(interaction) for interaction in interactions]
