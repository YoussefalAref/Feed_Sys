from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session

from app.database import get_db
from app.services import dashboard_service

router = APIRouter(tags=["dashboard"])


@router.get("/dashboard/stats")
@router.get("/manager/dashboard")
def get_dashboard_stats(db: Session = Depends(get_db)) -> dict:
    return dashboard_service.get_dashboard_stats(db)
