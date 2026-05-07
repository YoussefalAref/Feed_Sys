from app import models


def user_to_dict(user: models.User) -> dict:
    return {
        "id": user.id,
        "name": user.name,
        "email": user.email,
        "category": user.category,
        "score": user.score,
        "region": user.region,
    }


def product_to_dict(product: models.Item) -> dict:
    return {
        "id": product.id,
        "name": product.name,
        "price": product.price,
        "category": product.category,
        "popularity_score": product.popularity_score,
        "image": product.image,
        "description": product.description,
        "stock": product.stock,
    }


def interaction_to_dict(interaction: models.Interaction) -> dict:
    return {
        "id": interaction.id,
        "user_id": interaction.user_id,
        "item_id": interaction.item_id,
        "type": interaction.type,
        "timestamp": interaction.timestamp.isoformat(),
    }


def cart_item_to_dict(cart_item: models.CartItem) -> dict:
    return {
        "user_id": cart_item.user_id,
        "item_id": cart_item.item_id,
        "quantity": cart_item.quantity,
        "product": product_to_dict(cart_item.item),
    }
