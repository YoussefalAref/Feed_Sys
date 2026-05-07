from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session

from app.database import get_db
from app.services import recommendation_service

router = APIRouter(tags=["recommendations"])


@router.get("/recommendations/trending")
@router.get("/trending")
def get_trending(limit: int = 8, db: Session = Depends(get_db)) -> list[dict]:
    return recommendation_service.get_trending(db, limit)


@router.get("/recommendations/{user_id}")
def get_recommendations(
    user_id: int, limit: int = 4, db: Session = Depends(get_db)
) -> list[dict]:
    return recommendation_service.get_recommendations(db, user_id, limit)


@router.get("/products/{item_id}/related")
def get_related_products(
    item_id: int, limit: int = 3, db: Session = Depends(get_db)
) -> list[dict]:
    return recommendation_service.get_related_products(db, item_id, limit)
