from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session

from app.database import get_db
from app.schemas.interaction import InteractionInput
from app.services import interaction_service

router = APIRouter(prefix="/interactions", tags=["interactions"])


@router.post("")
def record_interaction(payload: InteractionInput, db: Session = Depends(get_db)) -> dict:
    return interaction_service.record_interaction(
        db,
        payload.user_id,
        payload.item_id,
        payload.type,
    )


@router.get("/recent")
def recent_interactions(limit: int = 8, db: Session = Depends(get_db)) -> list[dict]:
    return interaction_service.recent_interactions(db, limit)
