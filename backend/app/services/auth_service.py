"""
Authentication Service - Thin Python wrapper over C++ auth functions.

According to Backend API Contract:
- C++ authenticate_user(email, password) -> AuthResult
- C++ create_user(UserInput) -> AuthResult
- C++ get_user_by_id(user_id) -> UserDTO

This service:
1. Calls the C++ functions through pybind11
2. Falls back to SQLAlchemy if C++ module not available
3. Handles database persistence of user sessions (if needed)
4. Converts C++ DTOs to Python dicts for HTTP responses
"""

from fastapi import HTTPException
from sqlalchemy.orm import Session

from app import models
from app.services import cpp_core
from app.services.security import hash_password, verify_password
from app.services.serialization import user_to_dict


def make_token(user_id: int) -> str:
    """Generate a token for the user. In production, use JWT."""
    return f"dev-token-{user_id}"


def login(db: Session, email: str, password: str) -> dict:
    """
    Authenticate user by email and password.
    
    Contract: C++ authenticate_user(email, password) -> AuthResult
    
    Falls back to SQLAlchemy if C++ module not available.
    """
    core = cpp_core.load_core()
    
    if core and hasattr(core, 'authenticate_user'):
        try:
            # C++ function should return: {success: bool, error: str, user: UserDTO, token: str}
            result = core.authenticate_user(email, password)
            if not result.get('success'):
                raise HTTPException(status_code=401, detail=result.get('error', 'Invalid credentials'))
            return {
                "user": result.get('user'),
                "token": result.get('token', make_token(result.get('user', {}).get('id', 0)))
            }
        except Exception as e:
            # Fall through to database-backed implementation
            pass
    
    # Fallback: Database-backed authentication
    user = db.query(models.User).filter(models.User.email == email).first()
    if not user or not verify_password(password, user.password_hash):
        raise HTTPException(status_code=401, detail="Invalid email or password")

    return {"user": user_to_dict(user), "token": make_token(user.id)}


def signup(db: Session, payload: dict) -> dict:
    """
    Create a new user.
    
    Contract: C++ create_user(UserInput) -> AuthResult
    
    UserInput: {name, email, password, category, region}
    AuthResult: {success, error, user: UserDTO, token}
    
    Falls back to SQLAlchemy if C++ module not available.
    """
    core = cpp_core.load_core()
    
    if core and hasattr(core, 'create_user'):
        try:
            # C++ function expects UserInput dict
            result = core.create_user(payload)
            if not result.get('success'):
                if 'duplicate' in result.get('error', '').lower():
                    raise HTTPException(status_code=409, detail=result.get('error'))
                raise HTTPException(status_code=400, detail=result.get('error'))
            return {
                "user": result.get('user'),
                "token": result.get('token', make_token(result.get('user', {}).get('id', 0)))
            }
        except HTTPException:
            raise
        except Exception:
            # Fall through to database-backed implementation
            pass
    
    # Fallback: Database-backed user creation
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
    """
    Get user by ID.
    
    Contract: C++ get_user_by_id(user_id) -> UserDTO
    
    Falls back to SQLAlchemy if C++ module not available.
    """
    core = cpp_core.load_core()
    
    if core and hasattr(core, 'get_user_by_id'):
        try:
            result = core.get_user_by_id(user_id)
            if result is None:
                raise HTTPException(status_code=404, detail="User not found")
            return result
        except Exception:
            # Fall through to database-backed implementation
            pass
    
    # Fallback: Database-backed lookup
    user = db.get(models.User, user_id)
    if not user:
        raise HTTPException(status_code=404, detail="User not found")
    return user_to_dict(user)
