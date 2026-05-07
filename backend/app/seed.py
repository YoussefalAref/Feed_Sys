from datetime import datetime

from sqlalchemy import text

from app import models
from app.data.mock_data import MOCK_CART, MOCK_INTERACTIONS, MOCK_PRODUCTS, MOCK_USERS
from app.database import SessionLocal, engine, init_db
from app.services.security import hash_password


def seed() -> None:
    init_db()
    db = SessionLocal()
    try:
        if db.query(models.User).count() > 0:
            reset_postgres_sequences(db)
            print("Seed skipped: database already has users.")
            return

        for user in MOCK_USERS:
            db.add(
                models.User(
                    id=user["id"],
                    name=user["name"],
                    email=user["email"],
                    password_hash=hash_password(user["password"]),
                    category=user["category"],
                    score=user["score"],
                    region=user["region"],
                )
            )

        for product in MOCK_PRODUCTS:
            db.add(
                models.Item(
                    id=product["id"],
                    name=product["name"],
                    price=product["price"],
                    category=product["category"],
                    popularity_score=product["popularity_score"],
                    image=product["image"],
                    description=product["description"],
                    stock=product["stock"],
                )
            )

        for interaction in MOCK_INTERACTIONS:
            db.add(
                models.Interaction(
                    id=interaction["id"],
                    user_id=interaction["user_id"],
                    item_id=interaction["item_id"],
                    type=interaction["type"],
                    timestamp=datetime.fromisoformat(
                        interaction["timestamp"].replace("Z", "+00:00")
                    ).replace(tzinfo=None),
                )
            )

        for entry in MOCK_CART:
            db.add(
                models.CartItem(
                    user_id=entry["user_id"],
                    item_id=entry["item_id"],
                    quantity=entry["quantity"],
                )
            )

        db.commit()
        reset_postgres_sequences(db)
        print("Seed complete.")
    finally:
        db.close()


def reset_postgres_sequences(db) -> None:
    if engine.dialect.name != "postgresql":
        return

    sequence_updates = [
        "SELECT setval(pg_get_serial_sequence('users', 'id'), COALESCE(MAX(id), 1), true) FROM users",
        "SELECT setval(pg_get_serial_sequence('items', 'id'), COALESCE(MAX(id), 1), true) FROM items",
        "SELECT setval(pg_get_serial_sequence('interactions', 'id'), COALESCE(MAX(id), 1), true) FROM interactions",
    ]
    for statement in sequence_updates:
        db.execute(text(statement))
    db.commit()


if __name__ == "__main__":
    seed()
