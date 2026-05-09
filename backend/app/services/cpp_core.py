from __future__ import annotations

import importlib
import os
import sys
from functools import lru_cache
from pathlib import Path
from types import ModuleType


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[3]


def _candidate_dirs() -> list[Path]:
    configured_path = os.getenv("BITEAPPLE_CORE_PATH")
    candidates: list[Path] = []

    if configured_path:
        path = Path(configured_path)
        candidates.append(path if path.is_dir() else path.parent)

    build_dir = _repo_root() / "build"
    candidates.extend(path.parent for path in build_dir.glob("**/biteapple_core*.pyd"))
    candidates.extend(path.parent for path in build_dir.glob("**/biteapple_core*.so"))

    return candidates


def _missing(message: str):
    def _raiser(*_args, **_kwargs):
        raise NotImplementedError(message)

    return _raiser


@lru_cache(maxsize=1)
def load_core() -> ModuleType | None:
    cmod: ModuleType | None = None

    for candidate in _candidate_dirs():
        if not candidate.exists():
            continue

        candidate_str = str(candidate)
        if candidate_str not in sys.path:
            sys.path.insert(0, candidate_str)

        try:
            cmod = importlib.import_module("biteapple_core")
            break
        except ImportError:
            continue

    if cmod is None:
        try:
            cmod = importlib.import_module("biteapple_core")
        except ImportError:
            return None

    proxy = ModuleType("cpp_core_proxy")
    setattr(proxy, "_raw", cmod)

    contract_names = [
        "authenticate_user",
        "create_user",
        "get_user_by_id",
        "list_products",
        "get_product_by_id",
        "create_product",
        "update_product",
        "delete_product",
        "get_cart",
        "add_to_cart",
        "remove_from_cart",
        "checkout",
        "get_dashboard_stats",
        "record_interaction",
        "get_recent_interactions",
        "get_recommendations",
        "get_related_products",
        "get_trending",
        "rank_top_products",
        "score_recommendations",
    ]

    for name in contract_names:
        if hasattr(cmod, name):
            setattr(proxy, name, getattr(cmod, name))

    def _get_related_products(products, item_id, limit=3):
        if hasattr(cmod, "get_related_products"):
            return cmod.get_related_products(products, item_id, limit)
        raise NotImplementedError("C++ function 'get_related_products' not available")

    def _get_recent_interactions(interactions, limit=8):
        if hasattr(cmod, "get_recent_interactions"):
            return cmod.get_recent_interactions(interactions, limit)
        raise NotImplementedError("C++ function 'get_recent_interactions' not available")

    def _get_recommendations(products, interactions, user, limit=4):
        if hasattr(cmod, "score_recommendations"):
            return cmod.score_recommendations(products, interactions, user, limit)
        raise NotImplementedError("C++ function 'get_recommendations' not available")

    def _get_trending(products, limit=5):
        if hasattr(cmod, "rank_top_products"):
            return cmod.rank_top_products(products, limit)
        raise NotImplementedError("C++ function 'get_trending' not available")

    if not hasattr(proxy, "get_related_products"):
        setattr(proxy, "get_related_products", _get_related_products)
    if not hasattr(proxy, "get_recent_interactions"):
        setattr(proxy, "get_recent_interactions", _get_recent_interactions)
    if not hasattr(proxy, "get_recommendations"):
        setattr(proxy, "get_recommendations", _get_recommendations)
    if not hasattr(proxy, "get_trending"):
        setattr(proxy, "get_trending", _get_trending)

    if not hasattr(proxy, "rank_top_products"):
        setattr(proxy, "rank_top_products", _missing("C++ function 'rank_top_products' not available"))
    if not hasattr(proxy, "score_recommendations"):
        setattr(proxy, "score_recommendations", _missing("C++ function 'score_recommendations' not available"))

    for name in contract_names:
        if not hasattr(proxy, name):
            setattr(proxy, name, _missing(f"C++ function '{name}' not available"))

    return proxy


def is_available() -> bool:
    return load_core() is not None
