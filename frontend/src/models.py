import re
from datetime import datetime
from typing import Optional

from pydantic import BaseModel, EmailStr, Field, field_validator, model_validator


# ---------------------------------------------------------------------------
# Shared constants
# ---------------------------------------------------------------------------

# Password must be 8–72 chars and contain at least one uppercase letter,
# one lowercase letter, one digit, and one special character.
# 72-char cap matches bcrypt's input limit.
_PASSWORD_RE = re.compile(
    r"^(?=.*[a-z])(?=.*[A-Z])(?=.*\d)(?=.*[!@#$%^&*()_+\-=\[\]{};':\"\\|,.<>\/?]).{8,72}$"
)

# Valid category values (mirrors the C++ / DB enum)
VALID_CATEGORIES = {"USA-New", "USA-Active", "EU-New", "EU-Active", "APAC-New", "APAC-Active"}


# ---------------------------------------------------------------------------
# LoginRequest
# ---------------------------------------------------------------------------

class LoginRequest(BaseModel):
    """Payload sent by the frontend on the /auth/login endpoint."""

    email: EmailStr = Field(
        ...,
        description="Registered email address.",
        examples=["user@example.com"],
    )
    password: str = Field(
        ...,
        min_length=8,
        max_length=72,
        description="Plain-text password; the API will hash it before comparison.",
        examples=["P@ssw0rd!"],
    )

    @field_validator("email")
    @classmethod
    def normalise_email(cls, v: str) -> str:
        """Lowercase and strip whitespace so 'User@Example.com ' matches stored email."""
        return v.strip().lower()

    @field_validator("password")
    @classmethod
    def password_not_blank(cls, v: str) -> str:
        if not v.strip():
            raise ValueError("Password must not be blank or whitespace only.")
        return v

    model_config = {"json_schema_extra": {"example": {"email": "alice@biteapple.com", "password": "P@ssw0rd!"}}}


# ---------------------------------------------------------------------------
# SignupRequest
# ---------------------------------------------------------------------------

class SignupRequest(BaseModel):
    """Payload sent by the frontend on the /auth/signup endpoint."""

    email: EmailStr = Field(
        ...,
        description="New account email address. Must be unique.",
        examples=["newuser@example.com"],
    )
    password: str = Field(
        ...,
        min_length=8,
        max_length=72,
        description=(
            "Password must be 8–72 characters and include at least one uppercase letter, "
            "one lowercase letter, one digit, and one special character."
        ),
        examples=["P@ssw0rd1"],
    )
    confirm_password: str = Field(
        ...,
        description="Must match password exactly.",
        examples=["P@ssw0rd1"],
    )
    category: str = Field(
        ...,
        description=f"User segment. Allowed values: {sorted(VALID_CATEGORIES)}",
        examples=["USA-New"],
    )

    @field_validator("email")
    @classmethod
    def normalise_email(cls, v: str) -> str:
        return v.strip().lower()

    @field_validator("password")
    @classmethod
    def validate_password_strength(cls, v: str) -> str:
        if not _PASSWORD_RE.match(v):
            raise ValueError(
                "Password must be 8–72 characters and contain at least one uppercase letter, "
                "one lowercase letter, one digit, and one special character "
                "(!@#$%^&*()_+-=[]{}etc.)."
            )
        return v

    @field_validator("category")
    @classmethod
    def validate_category(cls, v: str) -> str:
        if v not in VALID_CATEGORIES:
            raise ValueError(f"Invalid category '{v}'. Must be one of: {sorted(VALID_CATEGORIES)}.")
        return v

    @model_validator(mode="after")
    def passwords_match(self) -> "SignupRequest":
        if self.password != self.confirm_password:
            raise ValueError("password and confirm_password do not match.")
        return self

    model_config = {
        "json_schema_extra": {
            "example": {
                "email": "alice@biteapple.com",
                "password": "P@ssw0rd1",
                "confirm_password": "P@ssw0rd1",
                "category": "USA-New",
            }
        }
    }


# ---------------------------------------------------------------------------
# UserResponse
# ---------------------------------------------------------------------------

class UserResponse(BaseModel):
    """Safe user representation returned to the frontend — never includes the password hash."""

    userId: int = Field(..., description="Unique user identifier.", examples=[42])
    email: EmailStr = Field(..., description="Registered email address.", examples=["alice@biteapple.com"])
    category: str = Field(..., description="User segment.", examples=["USA-Active"])
    score: float = Field(..., ge=0.0, description="User preference score (non-negative).", examples=[3.7])
    createdAt: str = Field(
        ...,
        description="Account creation timestamp in ISO 8601 format.",
        examples=["2024-01-15T10:30:00Z"],
    )

    @field_validator("createdAt")
    @classmethod
    def validate_iso_timestamp(cls, v: str) -> str:
        """Reject strings that are not valid ISO 8601 datetimes."""
        try:
            datetime.fromisoformat(v.replace("Z", "+00:00"))
        except ValueError:
            raise ValueError(f"createdAt must be a valid ISO 8601 timestamp, got '{v}'.")
        return v

    @classmethod
    def from_timestamp(cls, userId: int, email: str, category: str, score: float, ts: float) -> "UserResponse":
        """Convenience constructor that accepts a Unix timestamp instead of an ISO string."""
        return cls(
            userId=userId,
            email=email,
            category=category,
            score=score,
            createdAt=datetime.utcfromtimestamp(ts).strftime("%Y-%m-%dT%H:%M:%SZ"),
        )

    model_config = {
        "json_schema_extra": {
            "example": {
                "userId": 42,
                "email": "alice@biteapple.com",
                "category": "USA-Active",
                "score": 3.7,
                "createdAt": "2024-01-15T10:30:00Z",
            }
        }
    }


# ---------------------------------------------------------------------------
# AuthResponse
# ---------------------------------------------------------------------------

class AuthResponse(BaseModel):
    """
    Unified response envelope for /auth/login and /auth/signup.

    On success: success=True, userId and token are populated.
    On failure: success=False, message explains the reason, userId/token are None.
    """

    success: bool = Field(..., description="True if the operation succeeded.")
    message: str = Field(..., description="Human-readable status message.", examples=["Login successful."])
    userId: Optional[int] = Field(None, description="Authenticated user's ID. Null on failure.", examples=[42])
    token: Optional[str] = Field(
        None,
        description="Signed JWT for subsequent authenticated requests. Null on failure.",
        examples=["eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."],
    )

    # ── Convenience constructors ────────────────────────────────────────────

    @classmethod
    def ok(cls, message: str, userId: int, token: str) -> "AuthResponse":
        return cls(success=True, message=message, userId=userId, token=token)

    @classmethod
    def fail(cls, message: str) -> "AuthResponse":
        return cls(success=False, message=message)

    model_config = {
        "json_schema_extra": {
            "example": {
                "success": True,
                "message": "Login successful.",
                "userId": 42,
                "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
            }
        }
    }
