from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session

from app.database import get_db
from app.schemas.auth import LoginRequest, SignupRequest
from app.services import auth_service

router = APIRouter(prefix="/auth", tags=["auth"])
users_router = APIRouter(tags=["users"])


@router.post("/login")
def login(payload: LoginRequest, db: Session = Depends(get_db)) -> dict:
    return auth_service.login(db, payload.email, payload.password)


@router.post("/signup")
def signup(payload: SignupRequest, db: Session = Depends(get_db)) -> dict:
    return auth_service.signup(db, payload.model_dump())


@router.get("/me")
def me(user_id: int = 1, db: Session = Depends(get_db)) -> dict:
    return auth_service.get_user(db, user_id)


@users_router.get("/users/{user_id}")
def get_user(user_id: int, db: Session = Depends(get_db)) -> dict:
    return auth_service.get_user(db, user_id)
