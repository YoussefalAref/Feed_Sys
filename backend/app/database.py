import os

from dotenv import load_dotenv
from sqlalchemy import create_engine, inspect, text
from sqlalchemy.orm import DeclarativeBase, sessionmaker

load_dotenv()

DATABASE_URL = os.getenv("DATABASE_URL", "sqlite:///./biteapple_dev.db")

connect_args = {"check_same_thread": False} if DATABASE_URL.startswith("sqlite") else {}

engine = create_engine(DATABASE_URL, connect_args=connect_args)
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)


class Base(DeclarativeBase):
    pass


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


def init_db() -> None:
    from app import models

    Base.metadata.create_all(bind=engine)
    _apply_lightweight_schema_updates()


def _apply_lightweight_schema_updates() -> None:
    inspector = inspect(engine)
    if not inspector.has_table("orders"):
        return

    existing_columns = {column["name"] for column in inspector.get_columns("orders")}
    updates = []

    if "paymob_reference" not in existing_columns:
        updates.append("ALTER TABLE orders ADD COLUMN paymob_reference VARCHAR(255) DEFAULT ''")
    if "fawry_reference" not in existing_columns:
        updates.append("ALTER TABLE orders ADD COLUMN fawry_reference VARCHAR(255) DEFAULT ''")
    if "fulfillment_status" not in existing_columns:
        updates.append("ALTER TABLE orders ADD COLUMN fulfillment_status VARCHAR(50) DEFAULT 'not_fulfilled'")

    if not updates:
        return

    with engine.begin() as connection:
        for statement in updates:
            connection.execute(text(statement))
