# C++ / Python Integration Status

This document summarizes current integration status between the C++ core bindings and the Python FastAPI layer, what was changed to align with the contract, what's missing, and a suggested setup / push plan.

## What I changed

- `backend/app/services/cpp_core.py`
  - `load_core()` now returns a small proxy `ModuleType` that wraps the available C++ functions from the `biteapple_core` pybind module.
  - The proxy exposes:
    - `_raw` — the underlying C++ module (for debugging)
    - `get_recommendations(products, interactions, user, limit)` → delegates to C++ `score_recommendations` when available
    - `get_related_products(products, item_id, limit)` → delegates to C++ `get_related_products`
    - `get_recent_interactions(interactions, limit)` → delegates to C++ `get_recent_interactions`
    - `get_trending_via_rank(products, limit)` → delegates to C++ `rank_top_products`
  - If a C++ function is not present, the proxy raises `NotImplementedError` so the Python service falls back to DB-backed implementations.

- `backend/app/services/recommendation_service.py`
  - `get_trending()` now calls the proxy's `get_trending_via_rank(product_dicts, limit)` (if available) passing local product data, which matches the C++ `rank_top_products` signature.

- Added this document `docs/integration_status.md` describing the current state and next steps.

## Missing C++ functions (per contract)

The C++ binding (`src/bindings/biteapple_core.cpp`) currently exposes these functions:

- `rank_top_products` (exposed as `rank_top_products`) — ranking logic
- `get_related_products` — graph-based related product lookup
- `get_recent_interactions` — recent interactions slicing
- `score_recommendations` — recommendation scoring

Per the Backend API Contract, the Python services expect these C++ functions (not exhaustive):

Cluster 1 (Auth / Users):
- `authenticate_user`
- `create_user`
- `get_user_by_id`

Cluster 2 (Products / Cart / Dashboard):
- `list_products`
- `get_product_by_id`
- `create_product`
- `update_product`
- `delete_product`
- `get_cart`
- `add_to_cart`
- `remove_from_cart`
- `checkout`
- `get_dashboard_stats`

Cluster 3 (Interactions / Recommendations):
- `record_interaction`
- `get_recent_interactions` (IMPLEMENTED)
- `get_recommendations` → currently provided by `score_recommendations` (mapped)
- `get_related_products` (IMPLEMENTED)
- `get_trending` → available as `rank_top_products` (mapped via proxy)

Summary of missing items:
- Auth/user functions (all missing in C++ layer).
- Product CRUD and cart/checkout (missing in C++ layer).
- `record_interaction` (missing) — currently Python falls back to DB and enqueues interactions.

## Recommended next steps for parity with the contract

1. C++ team: implement and expose the missing functions in `biteapple_core` using the exact names in the contract, or document stable alternatives.
   - Prefer: expose functions with the same names used in the contract (e.g. `authenticate_user`, `create_user`, `get_user_by_id`, `list_products`, etc.)
   - For cart and checkout, ensure functions validate IDs and update product stock.
   - Implement `record_interaction` to update item popularity, user activity score, heap ranking, and graph weights.

2. Python team: keep FastAPI thin and call the C++ wrappers only. Current code already falls back to DB when C++ functions are unavailable — leave this behavior while C++ is completed.

3. Build/test: provide small C++ unit tests and a simple Python smoke test that imports the built Python extension and calls each exposed function with seed data.

4. CI: add a step to the pipeline that builds the C++ extension and runs the smoke tests before deploying APIs.

## How I validated changes locally

- Inspected `src/bindings/biteapple_core.cpp` and mapped currently implemented functions.
- Updated the Python proxy so services that already pass product lists to the proxy can call C++ ranking/scoring functions.

## How to finish and push

From the repository root, after C++ functions are implemented and `biteapple_core` is built (shared object placed under `build/` or available in `PYTHONPATH`):

```bash
# from project root
# build the C++ module using your existing build system (CMake / Make)
cmake -S . -B build && cmake --build build -j

# run Python unit/smoke tests
python -m pytest tests/

# commit and push
git add -A
git commit -m "Align Python wrapper with C++ binding contract; add integration status doc"
git push origin main
```

Notes:
- I did not push changes to the remote for you. I made repository edits in this branch (files updated locally).
- Once the C++ functions above are implemented and the extension is available, the Python layer will automatically use them via the proxy; otherwise it will continue to fall back to SQLAlchemy implementations.

If you want, I can:
- Add missing Python wrappers that currently call DB and raise clear errors so the C++ team has a concrete checklist.
- Open a PR with these changes and a checklist.
- Attempt to build the C++ extension here (requires compilers and dependencies); tell me if you want me to try building now.
