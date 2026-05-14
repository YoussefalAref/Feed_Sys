import logging
from datetime import UTC, datetime

from fastapi import HTTPException
from sqlalchemy.orm import Session

from app import models
from app.services import cpp_core
from app.services.serialization import interaction_to_dict

logger = logging.getLogger(__name__)

ALLOWED_TYPES = {"view", "click", "cart", "purchase"}

# Score weights mirror the C++ Queue → processInteractionQueue weights
_WEIGHTS = {"view": 1, "click": 2, "cart": 5, "purchase": 10}


def record_interaction(db: Session, user_id: int, item_id: int, interaction_type: str) -> dict:
    if interaction_type not in ALLOWED_TYPES:
        raise HTTPException(status_code=400, detail="Unsupported interaction type")

    user = db.get(models.User, user_id)
    product = db.get(models.Item, item_id)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    if not product:
        raise HTTPException(status_code=404, detail="Product not found")

    interaction = models.Interaction(
        user_id=user_id,
        item_id=item_id,
        type=interaction_type,
        timestamp=datetime.now(UTC).replace(tzinfo=None),
    )
    db.add(interaction)

    # Apply popularity weight to SQLite — keeps the DB in sync with what the
    # C++ Queue processes in memory (same weights, same logic).
    product.popularity_score = (product.popularity_score or 0) + _WEIGHTS.get(interaction_type, 0)

    db.commit()
    db.refresh(interaction)

    # Propagate through the C++ Queue so in-memory g_products stays consistent
    # with SQLite (used by recommendations and trending).
    core = cpp_core.load_core()
    if core:
        try:
            core.record_interaction(user_id, item_id, interaction_type)
        except Exception:
            logger.debug("C++ record_interaction skipped (item not in C++ seed)", exc_info=True)

    return interaction_to_dict(interaction)


def recent_interactions(db: Session, limit: int = 8) -> list[dict]:
    interactions = (
        db.query(models.Interaction)
        .order_by(models.Interaction.timestamp.desc(), models.Interaction.id.desc())
        .limit(limit)
        .all()
    )
    return [interaction_to_dict(interaction) for interaction in interactions]
