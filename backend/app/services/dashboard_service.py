from sqlalchemy import func
from sqlalchemy.orm import Session

from app import models
from app.services import cpp_core


def get_dashboard_stats(db: Session) -> dict:
    core = cpp_core.load_core()
    if core and hasattr(core, "get_dashboard_stats"):
        try:
            result = core.get_dashboard_stats()
            if result is not None:
                return result
        except Exception:
            pass

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
