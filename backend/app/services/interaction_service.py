from datetime import UTC, datetime

from fastapi import HTTPException
from sqlalchemy.orm import Session

from app import models
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
    interactions = (
        db.query(models.Interaction)
        .order_by(models.Interaction.timestamp.desc(), models.Interaction.id.desc())
        .limit(limit)
        .all()
    )
    return [interaction_to_dict(interaction) for interaction in interactions]
