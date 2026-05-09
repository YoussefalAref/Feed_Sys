"""
Product Service - Thin Python wrapper over C++ product functions.

According to Backend API Contract:
- C++ list_products(category) -> std::vector<ProductDTO>
- C++ get_product_by_id(item_id) -> ProductDTO
- C++ create_product(ProductInput) -> ProductDTO
- C++ update_product(item_id, ProductInput) -> ProductDTO
- C++ delete_product(item_id) -> bool
- C++ get_trending(limit) -> std::vector<ProductDTO>

This service:
1. Calls C++ functions through pybind11
2. Falls back to SQLAlchemy if C++ module not available
3. Converts C++ DTOs to Python dicts for HTTP responses
"""

from fastapi import HTTPException
from sqlalchemy.orm import Session

from app import models
from app.services import cpp_core
from app.services.serialization import product_to_dict


def list_products(db: Session, category: str | None = None) -> list[dict]:
    """
    List products, optionally filtered by category.
    
    Contract: C++ list_products(category) -> std::vector<ProductDTO>
    """
    core = cpp_core.load_core()
    
    if core and hasattr(core, 'list_products'):
        try:
            category_str = category or ""
            products = core.list_products(category_str)
            if isinstance(products, list):
                return products
        except Exception:
            # Fall through to database-backed implementation
            pass
    
    # Fallback: Database-backed query
    query = db.query(models.Item)
    if category:
        query = query.filter(models.Item.category.ilike(category))
    return [product_to_dict(product) for product in query.order_by(models.Item.id).all()]


def get_product(db: Session, item_id: int) -> dict:
    """
    Get product by ID.
    
    Contract: C++ get_product_by_id(item_id) -> ProductDTO
    """
    core = cpp_core.load_core()
    
    if core and hasattr(core, 'get_product_by_id'):
        try:
            product = core.get_product_by_id(item_id)
            if product is None:
                raise HTTPException(status_code=404, detail="Product not found")
            return product
        except Exception:
            # Fall through to database-backed implementation
            pass
    
    # Fallback: Database-backed lookup
    product = db.get(models.Item, item_id)
    if not product:
        raise HTTPException(status_code=404, detail="Product not found")
    return product_to_dict(product)


def create_product(db: Session, payload: dict) -> dict:
    """
    Create a new product.
    
    Contract: C++ create_product(ProductInput) -> ProductDTO
    
    ProductInput: {name, price, category, stock, image, description, popularity_score}
    """
    core = cpp_core.load_core()
    
    if core and hasattr(core, 'create_product'):
        try:
            product = core.create_product(payload)
            if product is not None:
                return product
        except Exception:
            # Fall through to database-backed implementation
            pass
    
    # Fallback: Database-backed creation
    product = models.Item(
        name=payload["name"],
        price=float(payload["price"]),
        category=payload["category"],
        stock=int(payload.get("stock", 0)),
        image=payload.get("image", ""),
        description=payload.get("description", ""),
        popularity_score=float(payload.get("popularity_score", 40)),
    )
    db.add(product)
    db.commit()
    db.refresh(product)
    return product_to_dict(product)


def update_product(db: Session, item_id: int, payload: dict) -> dict:
    """
    Update an existing product.
    
    Contract: C++ update_product(item_id, ProductInput) -> ProductDTO
    """
    core = cpp_core.load_core()
    
    if core and hasattr(core, 'update_product'):
        try:
            product = core.update_product(item_id, payload)
            if product is not None:
                return product
        except Exception:
            # Fall through to database-backed implementation
            pass
    
    # Fallback: Database-backed update
    product = db.get(models.Item, item_id)
    if not product:
        raise HTTPException(status_code=404, detail="Product not found")

    product.name = payload["name"]
    product.price = float(payload["price"])
    product.category = payload["category"]
    product.stock = int(payload.get("stock", product.stock))
    product.image = payload.get("image", product.image)
    product.description = payload.get("description", product.description)
    product.popularity_score = float(payload.get("popularity_score", product.popularity_score))
    db.commit()
    db.refresh(product)
    return product_to_dict(product)


def delete_product(db: Session, item_id: int) -> dict:
    """
    Delete a product.
    
    Contract: C++ delete_product(item_id) -> bool
    """
    core = cpp_core.load_core()
    
    if core and hasattr(core, 'delete_product'):
        try:
            success = core.delete_product(item_id)
            if success:
                # Also clean up database rows for consistency
                db.query(models.CartItem).filter(models.CartItem.item_id == item_id).delete()
                db.query(models.Interaction).filter(models.Interaction.item_id == item_id).delete()
                db.commit()
                return {"success": True}
        except Exception:
            # Fall through to database-backed implementation
            pass
    
    # Fallback: Database-backed deletion
    product = db.get(models.Item, item_id)
    if not product:
        raise HTTPException(status_code=404, detail="Product not found")

    db.query(models.CartItem).filter(models.CartItem.item_id == item_id).delete()
    db.query(models.Interaction).filter(models.Interaction.item_id == item_id).delete()
    db.delete(product)
    db.commit()
    return {"success": True}


def get_trending(db: Session, limit: int = 8) -> list[dict]:
    """
    Get trending products (highest popularity).
    
    Contract: C++ get_trending(limit) -> std::vector<ProductDTO>
    """
    core = cpp_core.load_core()
    
    if core and hasattr(core, 'get_trending'):
        try:
            products = core.get_trending(limit)
            if isinstance(products, list):
                return products
        except Exception:
            # Fall through to database-backed implementation
            pass
    
    # Fallback: Database-backed trending
    products = (
        db.query(models.Item)
        .order_by(models.Item.popularity_score.desc(), models.Item.id.asc())
        .limit(limit)
        .all()
    )
    return [product_to_dict(product) for product in products]
