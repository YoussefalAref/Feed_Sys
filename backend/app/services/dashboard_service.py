"""
Dashboard Service - Thin Python wrapper over C++ dashboard function.

According to Backend API Contract:
- C++ get_dashboard_stats() -> DashboardStats

This service:
1. Calls C++ function through pybind11
2. Falls back to SQLAlchemy if C++ module not available
3. Converts C++ DTO to Python dict for HTTP response
"""

from sqlalchemy import func
from sqlalchemy.orm import Session

from app import models
from app.services import cpp_core


def get_dashboard_stats(db: Session) -> dict:
    """
    Get dashboard statistics.
    
    Contract: C++ get_dashboard_stats() -> DashboardStats
    Returns: {totalProducts, totalUsers, totalInteractions, mostPopularCategory}
    """
    core = cpp_core.load_core()
    
    if core and hasattr(core, 'get_dashboard_stats'):
        try:
            stats = core.get_dashboard_stats()
            if isinstance(stats, dict):
                return stats
        except Exception:
            # Fall through to database-backed implementation
            pass
    
    # Fallback: Database-backed statistics
    category_counts = (
        db.query(models.Item.category, func.sum(models.Item.popularity_score))
        .group_by(models.Item.category)
        .all()
    )
    most_popular_category = "N/A"
    if category_counts:
        most_popular_category = max(category_counts, key=lambda item: item[1] or 0)[0]

    return {
        "totalProducts": db.query(models.Item).count(),
        "totalUsers": db.query(models.User).count(),
        "totalInteractions": db.query(models.Interaction).count(),
        "mostPopularCategory": most_popular_category,
    }
