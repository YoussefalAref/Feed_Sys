"""
C++ Core Binding Module Wrapper

This module provides a unified interface to the C++ core functions exposed via pybind11.
It handles loading the biteapple_core module and provides wrapper functions that match
the contract specification.

C++ Functions Expected (from Backend API Contract):
- Cluster 1 (Auth): authenticate_user, create_user, get_user_by_id
- Cluster 2 (Products/Cart): list_products, get_product_by_id, create_product, update_product,
  delete_product, get_cart, add_to_cart, remove_from_cart, checkout, get_dashboard_stats
- Cluster 3 (Interactions): record_interaction, get_recent_interactions, get_recommendations,
  get_related_products, get_trending
"""

from __future__ import annotations

import importlib
import os
import sys
from functools import lru_cache
from pathlib import Path
from types import ModuleType


def _repo_root() -> Path:
    """Get the repository root directory."""
    return Path(__file__).resolve().parents[3]


def _candidate_dirs() -> list[Path]:
    """Get candidate directories where biteapple_core module might be located."""
    configured_path = os.getenv("BITEAPPLE_CORE_PATH")
    candidates: list[Path] = []

    if configured_path:
        path = Path(configured_path)
        candidates.append(path if path.is_dir() else path.parent)

    build_dir = _repo_root() / "build"
    candidates.extend(path.parent for path in build_dir.glob("**/biteapple_core*.pyd"))
    candidates.extend(path.parent for path in build_dir.glob("**/biteapple_core*.so"))

    return candidates


@lru_cache(maxsize=1)
def load_core() -> ModuleType | None:
    """
    Dynamically load the biteapple_core pybind11 module.
    Returns None if the module is not available.
    """
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
            cmod = None
            continue

    if cmod is None:
        try:
            cmod = importlib.import_module("biteapple_core")
        except ImportError:
            return None

    # Build a small proxy module that maps the expected Python-facing names
    # (as used by FastAPI services) to the functions exposed by the C++ binding.
    proxy = ModuleType("cpp_core_proxy")

    # Expose the raw underlying module for advanced use or debugging
    setattr(proxy, "_raw", cmod)

    # Mapping helpers
    def _wrap_score_recommendations(products, interactions, user, limit=4):
        if hasattr(cmod, "score_recommendations"):
            return cmod.score_recommendations(products, interactions, user, limit)
        raise NotImplementedError("C++ function 'score_recommendations' not available")

    def _wrap_get_related_products(products, item_id, limit=3):
        if hasattr(cmod, "get_related_products"):
            return cmod.get_related_products(products, item_id, limit)
        raise NotImplementedError("C++ function 'get_related_products' not available")

    def _wrap_get_recent_interactions(interactions, limit=8):
        if hasattr(cmod, "get_recent_interactions"):
            return cmod.get_recent_interactions(interactions, limit)
        raise NotImplementedError("C++ function 'get_recent_interactions' not available")

    def _wrap_rank_top_products(products, limit=5):
        if hasattr(cmod, "rank_top_products"):
            return cmod.rank_top_products(products, limit)
        raise NotImplementedError("C++ function 'rank_top_products' not available")

    # Attach thin wrappers with the names services expect.
    setattr(proxy, "get_recommendations", _wrap_score_recommendations)
    setattr(proxy, "get_related_products", _wrap_get_related_products)
    setattr(proxy, "get_recent_interactions", _wrap_get_recent_interactions)
    # For trending, prefer rank_top_products which requires a product list
    setattr(proxy, "get_trending_via_rank", _wrap_rank_top_products)

    return proxy


def is_available() -> bool:
    """Check if the C++ core module is available."""
    return load_core() is not None
