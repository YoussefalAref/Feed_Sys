from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.orm import Session

from app.database import get_db
from app.services import auth_service
from app.schemas.auth import LoginRequest, SignupRequest

router = APIRouter(prefix="/auth")


@router.post("/login")
def login(req: LoginRequest, db: Session = Depends(get_db)):
    try:
        return auth_service.login(db, req.email, req.password)
    except HTTPException as e:
        raise e


@router.post("/signup")
def signup(req: SignupRequest, db: Session = Depends(get_db)):
    try:
        return auth_service.signup(db, req.dict())
    except HTTPException as e:
        raise e


@router.get("/users/{user_id}")
def get_user(user_id: int, db: Session = Depends(get_db)) -> dict:
    return auth_service.get_user(db, user_id)
