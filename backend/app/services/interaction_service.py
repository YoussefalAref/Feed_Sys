from datetime import UTC, datetime

from fastapi import HTTPException
from sqlalchemy.orm import Session

from app import models
from app.services import cpp_core
from app.services.serialization import interaction_to_dict

ALLOWED_TYPES = {"view", "click", "cart", "purchase"}


def record_interaction(db: Session, user_id: int, item_id: int, interaction_type: str) -> dict:
    if interaction_type not in ALLOWED_TYPES:
        raise HTTPException(status_code=400, detail="Unsupported interaction type")

    user = db.get(models.User, user_id)
    product = db.get(models.Item, item_id)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    if not product:
        raise HTTPException(status_code=404, detail="Product not found")

    core = cpp_core.load_core()
    if core and hasattr(core, "record_interaction"):
        try:
            result = core.record_interaction(user_id, item_id, interaction_type)
            if result is not None:
                return result
        except Exception:
            pass

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


def recent_interactions(db: Session, limit: int = 8) -> list[dict]:
    core = cpp_core.load_core()
    if core:
        interactions = (
            db.query(models.Interaction)
            .order_by(models.Interaction.timestamp.asc(), models.Interaction.id.asc())
            .all()
        )
        return core.get_recent_interactions(
            [interaction_to_dict(interaction) for interaction in interactions],
            limit,
        )

    interactions = (
        db.query(models.Interaction)
        .order_by(models.Interaction.timestamp.desc(), models.Interaction.id.desc())
        .limit(limit)
        .all()
    )
    return [interaction_to_dict(interaction) for interaction in interactions]


def get_recent_interactions(db: Session, limit: int = 8) -> list[dict]:
    return recent_interactions(db, limit)
