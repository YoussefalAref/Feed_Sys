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
    candidates.extend(path.parent for path in build_dir.glob("phase6/**/biteapple_core*.pyd"))
    candidates.extend(path.parent for path in build_dir.glob("phase6/**/biteapple_core*.so"))

    return candidates


@lru_cache(maxsize=1)
def load_core() -> ModuleType | None:
    for candidate in _candidate_dirs():
        if not candidate.exists():
            continue

        candidate_str = str(candidate)
        if candidate_str not in sys.path:
            sys.path.insert(0, candidate_str)

        try:
            return importlib.import_module("biteapple_core")
        except ImportError:
            continue

    try:
        return importlib.import_module("biteapple_core")
    except ImportError:
        return None


def is_available() -> bool:
    return load_core() is not None
