from argon2 import PasswordHasher
from argon2.exceptions import VerifyMismatchError

# Use secure Argon2 parameters (argon2-cffi default is reasonable)
_ph = PasswordHasher()


def hash_password(password: str) -> str:
    return _ph.hash(password)


def verify_password(password: str, password_hash: str) -> bool:
    try:
        return _ph.verify(password_hash, password)
    except VerifyMismatchError:
        return False
    except Exception:
        return False
