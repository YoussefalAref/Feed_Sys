from fastapi import HTTPException
from sqlalchemy.orm import Session

from app import models
from app.services import cpp_core
from app.services.security import hash_password, verify_password
from app.services.serialization import user_to_dict


def make_token(user_id: int) -> str:
    return f"mock-token-{user_id}"


def login(db: Session, email: str, password: str) -> dict:
    core = cpp_core.load_core()
    if core and hasattr(core, "authenticate_user"):
        try:
            result = core.authenticate_user(email, password)
            if result is not None:
                return result
        except Exception:
            pass

    user = db.query(models.User).filter(models.User.email == email).first()
    if not user or not verify_password(password, user.password_hash):
        raise HTTPException(status_code=401, detail="Invalid email or password")

    return {"user": user_to_dict(user), "token": make_token(user.id)}


def signup(db: Session, payload: dict) -> dict:
    core = cpp_core.load_core()
    if core and hasattr(core, "create_user"):
        try:
            result = core.create_user(payload)
            if result is not None:
                return result
        except Exception:
            pass

    exists = db.query(models.User).filter(models.User.email == payload["email"]).first()
    if exists:
        raise HTTPException(status_code=409, detail="Email is already registered")

    user = models.User(
        name=payload["name"],
        email=payload["email"],
        password_hash=hash_password(payload["password"]),
        category=payload.get("category") or "Electronics",
        score=50,
        region=payload.get("region") or "Cairo",
    )
    db.add(user)
    db.commit()
    db.refresh(user)
    return {"user": user_to_dict(user), "token": make_token(user.id)}


def get_user(db: Session, user_id: int) -> dict:
    core = cpp_core.load_core()
    if core and hasattr(core, "get_user_by_id"):
        try:
            result = core.get_user_by_id(user_id)
            if result is not None:
                return result
        except Exception:
            pass

    user = db.get(models.User, user_id)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    return user_to_dict(user)
