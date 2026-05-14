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
    candidates = []
    configured_path = os.getenv("BITEAPPLE_CORE_PATH")
    if configured_path:
        path = Path(configured_path)
        if path.is_dir():
            candidates.append(path)
    build_dir = _repo_root() / "build"
    if build_dir.is_dir():
        candidates.append(build_dir)
    candidates.extend(path.parent for path in build_dir.glob("**/biteapple_core*.pyd"))
    candidates.extend(path.parent for path in build_dir.glob("**/biteapple_core*.so"))
    return candidates


def _missing(message: str):
    def _raiser(*_args, **_kwargs):
        raise NotImplementedError(message)
    return _raiser


@lru_cache(maxsize=1)
def load_core() -> ModuleType | None:
    for candidate in _candidate_dirs():
        if not candidate.exists():
            continue
        candidate_str = str(candidate)
        if candidate_str not in sys.path:
            sys.path.insert(0, candidate_str)
        try:
            cmod = importlib.import_module("biteapple_core")
        except ImportError:
            continue

        proxy = ModuleType("cpp_core_proxy")
        proxy._raw = cmod  # type: ignore[attr-defined]

        contract_names = (
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
        )

        def _get_related_products(products, item_id, limit):
            if hasattr(cmod, "get_related_products"):
                return cmod.get_related_products(products, item_id, limit)
            raise NotImplementedError("C++ function 'get_related_products' not available")

        def _get_recent_interactions(interactions, limit):
            if hasattr(cmod, "get_recent_interactions"):
                return cmod.get_recent_interactions(interactions, limit)
            raise NotImplementedError("C++ function 'get_recent_interactions' not available")

        def _get_recommendations(products, interactions, user, limit):
            if hasattr(cmod, "score_recommendations"):
                return cmod.score_recommendations(products, interactions, user, limit)
            raise NotImplementedError("C++ function 'get_recommendations' not available")

        def _get_trending(products, limit):
            if hasattr(cmod, "rank_top_products"):
                return cmod.rank_top_products(products, limit)
            raise NotImplementedError("C++ function 'get_trending' not available")

        setattr(proxy, "get_related_products", _get_related_products)
        setattr(proxy, "get_recent_interactions", _get_recent_interactions)
        setattr(proxy, "get_recommendations", _get_recommendations)
        setattr(proxy, "get_trending", _get_trending)

        for name in contract_names:
            if not hasattr(proxy, name):
                if hasattr(cmod, name):
                    setattr(proxy, name, getattr(cmod, name))
                else:
                    setattr(proxy, name, _missing(f"C++ function '{name}' not available"))

        return proxy

    return None


def is_available() -> bool:
    return bool(load_core())
