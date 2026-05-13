from datetime import datetime, timedelta
import os
from typing import Optional

import jwt

# Secret and algorithm: in production, store the secret in environment or secrets manager
_SECRET = os.getenv("JWT_SECRET", "dev-secret-change-me")
_ALGORITHM = os.getenv("JWT_ALGORITHM", "HS256")
_ACCESS_EXPIRE_MINUTES = int(os.getenv("JWT_ACCESS_EXPIRE_MINUTES", "60"))


def create_access_token(subject: str, expires_delta: Optional[timedelta] = None) -> str:
    now = datetime.utcnow()
    if expires_delta is None:
        expires_delta = timedelta(minutes=_ACCESS_EXPIRE_MINUTES)
    payload = {
        "sub": str(subject),
        "iat": now,
        "exp": now + expires_delta,
    }
    token = jwt.encode(payload, _SECRET, algorithm=_ALGORITHM)
    # PyJWT returns str in v2+
    if isinstance(token, bytes):
        token = token.decode("utf-8")
    return token


def decode_access_token(token: str) -> dict:
    return jwt.decode(token, _SECRET, algorithms=[_ALGORITHM])
