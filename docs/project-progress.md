# BiteApple Project Progress Dashboard

Use this file as the working control panel for the project. Update the checkboxes
after each development session so everyone can see the current phase, what is
done, what is blocked, and what should happen next.

## Status Legend

| Mark | Meaning |
|---|---|
| `[x]` | Done and verified |
| `[~]` | In progress or partially verified |
| `[ ]` | Not started |
| `BLOCKED` | Needs another layer, dependency, or decision first |

## Current Phase

| Current Focus | Status | Next Action |
|---|---|---|
| Phase 0: Test and stabilize frontend | `[x]` | Recheck after frontend API swap |
| Phase 1: Freeze API contract | `[~]` | Reconcile endpoint names before Phase 3 if desired |
| Phase 2: Dummy FastAPI backend | `[x]` | Use it as the Phase 3 target |
| Phase 3: Connect frontend to dummy FastAPI | `[x]` | Use as the working full-stack dev setup |
| Phase 4: Add PostgreSQL persistence | `[~]` | Code is implemented; follow `docs/environment-setup.md` on target machine |
| Phase 4.5: Add Fawry payment flow | `[~]` | Mock mode works; real sandbox needs merchant credentials |
| Phase 5: Build C++ ADS core separately | `[ ]` | C++ team owns this; use `docs/cpp-core-function-contract.md` |
| Phase 6: Add pybind integration | `[ ]` | Start after C++ core functions are delivered |
| Phase 7: Use C++ inside FastAPI | `[ ]` | Start after pybind module passes smoke tests |

## Golden Rules

- Keep all frontend API communication inside `frontend/src/services/api.js`.
- Do not scatter `fetch` calls inside pages or components.
- Do not connect frontend directly to C++ or PostgreSQL.
- Keep FastAPI response shapes stable once the frontend uses them.
- Add layers in this order: frontend stability, API contract, dummy backend,
  frontend fetch swap, PostgreSQL, Fawry payment flow, C++ core, pybind,
  C++ integration, final demo.

## Phase 0: Test And Stabilize Frontend

Goal: make the current React app run, build, route, and use `api.js` cleanly
before backend work begins.

| Task | Status | Evidence / Notes |
|---|---|---|
| Run `npm install` in `frontend` | `[x]` | Verified with `npm.cmd install` |
| Run user dev server | `[x]` | User UI responded on `127.0.0.1:5173` |
| Run admin dev server | `[x]` | Admin UI responded on `127.0.0.1:5174` |
| Run `npm run build` | `[x]` | Verified with `npm.cmd run build` |
| Run admin build | `[x]` | Verified with `npm.cmd run build:admin` |
| Confirm routes render | `[x]` | Checked home, login, signup, cart, product details, admin |
| Confirm no direct component `fetch` calls | `[x]` | Search found none outside service layer |
| Confirm pages/components use `api.js` boundary | `[x]` | Mock data only imported by `services/api.js` |
| Confirm localStorage login/logout | `[x]` | Login sets user/token; logout clears both |
| Fix runtime console errors | `[x]` | Added inline favicon to remove favicon 404 |
| Review remaining React Router warnings | `[ ]` | Warnings are not blocking, can defer |

Phase 0 Done When:

- [x] Frontend opens without crashing.
- [x] Production build succeeds.
- [x] Login/signup works with mock data.
- [x] Product feed and product details show.
- [x] Cart and dashboard open.
- [x] No component bypasses `services/api.js`.

## Phase 1: Freeze API Contract

Goal: write and agree on the exact frontend-to-FastAPI contract before building
the backend.

| Task | Status | Evidence / Notes |
|---|---|---|
| Keep current frontend service function list documented | `[x]` | See `docs/frontend-api-integration-checklist.md` |
| Keep backend/C++ binding contract documented | `[x]` | See `docs/backend-api-cpp-contract.md` |
| Decide final endpoint naming style | `[~]` | Existing docs differ on `/manager/*` vs `/dashboard/stats` style |
| Confirm auth response shape | `[x]` | user + token |
| Confirm product response shape | `[x]` | id, name, price, category, score, stock, image, description |
| Confirm interaction response shape | `[x]` | id, user_id, item_id, type, timestamp |
| Confirm cart response shape | `[x]` | cart row with nested product |
| Confirm dashboard stats shape | `[x]` | totalProducts, totalUsers, totalInteractions, mostPopularCategory |
| Confirm checkout/payment response shape | `[x]` | order id, payment status, Fawry reference, payment attempt |
| Create final API contract file if needed | `[ ]` | Optional if existing contract is accepted |

Phase 1 Done When:

- [ ] One endpoint list is treated as the source of truth.
- [ ] Every `api.js` function maps to a backend endpoint.
- [ ] Request and response bodies are stable.
- [ ] Checkout and payment status responses are stable.
- [ ] Error status codes are documented.

## Phase 2: Build Dummy FastAPI Backend

Goal: create real HTTP endpoints backed by in-memory mock data only.

| Task | Status | Evidence / Notes |
|---|---|---|
| Create `backend/` app structure | `[x]` | main, routes, schemas, services, mock data |
| Add FastAPI dependencies | `[x]` | `backend/requirements.txt` |
| Add CORS for React ports | `[x]` | Includes `localhost/127.0.0.1:5173` and `5174` |
| Implement auth routes | `[x]` | login, signup, me, user lookup |
| Implement product routes | `[x]` | list, detail, category, manager CRUD |
| Implement interaction routes | `[x]` | record and recent |
| Implement recommendation routes | `[x]` | recommendations, related products, trending |
| Implement cart routes | `[x]` | get, add, remove, checkout |
| Implement dashboard route | `[x]` | `/manager/dashboard` and `/dashboard/stats` |
| Verify Swagger `/docs` opens | `[x]` | `http://127.0.0.1:8000/docs` returned 200 |

Phase 2 Done When:

- [x] Backend runs with `uvicorn`.
- [x] `/docs` opens.
- [x] All endpoints return frontend-compatible JSON.
- [x] No PostgreSQL, C++, or pybind is required yet.

## Phase 3: Connect Frontend To Dummy FastAPI

Goal: replace mock internals in `frontend/src/services/api.js` with `fetch`
calls while keeping page/component imports unchanged.

| Task | Status | Evidence / Notes |
|---|---|---|
| Add `API_BASE_URL` env fallback | `[x]` | Uses `VITE_API_BASE_URL` or `http://127.0.0.1:8000` |
| Add JSON request helper | `[x]` | Centralized fetch/error handling in `api.js` |
| Replace auth mock calls | `[x]` | login/signup call FastAPI; logout remains local cleanup |
| Replace product calls | `[x]` | list/detail/category/CRUD |
| Replace interaction calls | `[x]` | record/recent |
| Replace recommendation calls | `[x]` | recommendations/related |
| Replace cart calls | `[x]` | get/add/remove/checkout |
| Replace dashboard stats call | `[x]` | Same returned shape |
| Re-run direct fetch search | `[x]` | Direct fetch only exists in `frontend/src/services/api.js` |

Phase 3 Done When:

- [x] Frontend works against FastAPI.
- [x] Pages/components still import the same service names.
- [x] No direct component HTTP calls exist.

## Phase 4: Add PostgreSQL Persistence

Goal: replace backend in-memory data with persistent storage while preserving
the same API contract.

| Task | Status | Evidence / Notes |
|---|---|---|
| Add DB config and env variables | `[x]` | `DATABASE_URL`, `.env.example`, SQLite fallback |
| Add SQLAlchemy connection/session layer | `[x]` | `backend/app/database.py` |
| Add users table/model | `[x]` | Include password hash, category, score, region |
| Add items table/model | `[x]` | Product inventory data |
| Add interactions table/model | `[x]` | view/click/cart/purchase events |
| Add cart table/model | `[x]` | user_id, item_id, quantity |
| Add orders table/model | `[x]` | order id, user id, total, payment status |
| Add payment attempts table/model | `[x]` | provider, reference, status, raw response |
| Add seed data | `[x]` | `python -m app.seed` |
| Add password hashing | `[x]` | stdlib PBKDF2 |
| Replace mock services with DB queries | `[x]` | Same endpoints and response shapes |

Phase 4 Done When:

- [x] Data persists after backend restart.
- [x] Frontend still works without route/shape changes.
- [x] Login uses hashed passwords.

Note: verified with the local SQLite fallback because PostgreSQL is not installed
or configured on this machine. The same SQLAlchemy layer is ready for PostgreSQL
when `DATABASE_URL` is set.
See `docs/environment-setup.md` for the target-machine setup steps.

## Phase 4.5: Add Paymob Payment Flow

Goal: connect cart checkout to a Paymob payment workflow through FastAPI while
keeping React dependent only on `frontend/src/services/api.js`.

This belongs after PostgreSQL because payment needs persistent orders, payment
attempts, and status tracking. It should happen before final demo polish, and it
does not require C++.

| Task | Status | Evidence / Notes |
|---|---|---|
| Choose checkout API shape | `[x]` | Checkout creates order, starts Paymob reference, status endpoint checks payment |
| Add Paymob config env variables | `[x]` | `.env.example`; sandbox secrets stay out of Git |
| Add backend payment service | `[x]` | FastAPI talks to Paymob; React only calls FastAPI |
| Connect cart checkout route to order creation | `[x]` | Cart snapshot saved to `orders` and `order_items` |
| Store Paymob payment reference | `[x]` | Stored on order and payment attempt |
| Add payment status endpoint | `[x]` | `/orders/{order_id}/payment` and `/payments/orders/{order_id}/status` |
| Handle failed or cancelled payment | `[x]` | Status normalization keeps cart unfulfilled unless paid |
| Update `api.js` checkout/payment functions | `[x]` | Checkout, status, and mock-paid helpers |
| Add sandbox/manual test notes | `[~]` | Mock mode verified; real sandbox waits for credentials |

Phase 4.5 Done When:

- [x] A cart checkout creates a persistent order.
- [x] Backend can start a Fawry payment attempt.
- [x] Payment status is stored and readable by the frontend.
- [x] Failed payment does not incorrectly clear or mark the order paid.
- [x] No payment secrets are committed.

Note: local verification uses `PAYMOB_MOCK_MODE=true`. Real Paymob sandbox
verification needs the Paymob credentials from the sandbox dashboard.

## Phase 5: Build C++ ADS Core Separately

Goal: implement and test the ADS logic independently from frontend and FastAPI.

| Task | Status | Evidence / Notes |
|---|---|---|
| Confirm C++ folder structure | `[ ]` | C++ team to provide; see contract doc |
| Implement user/item/interaction models | `[ ]` | Match DTO fields from contract |
| Implement HashMap lookup layer | `[ ]` | Dashboard/category lookup support |
| Implement interaction queue | `[ ]` | Recent events in order |
| Implement ranking heap/top-K | `[ ]` | `rank_top_products(products, limit)` |
| Implement graph related-products logic | `[ ]` | `get_related_products(products, item_id, limit)` |
| Implement recommendation scoring | `[ ]` | `get_recommendations(user, products, interactions, limit)` |
| Add C++ tests or demo driver | `[ ]` | Must verify all contract functions |

Phase 5 Done When:

- [ ] C++ builds independently.
- [ ] Data-structure behavior is demonstrable.
- [ ] Recommendation/ranking examples work without Python.

## Phase 6: Add pybind Integration

Goal: expose tested C++ functions to Python.

| Task | Status | Evidence / Notes |
|---|---|---|
| Add pybind dependency/build config | `[ ]` | Add only after C++ core exists |
| Create binding module | `[ ]` | Required module name: `biteapple_core` |
| Expose ranking functions | `[ ]` | `rank_top_products(products, limit)` |
| Expose interaction functions | `[ ]` | `get_recent_interactions(interactions, limit)` |
| Expose related-products functions | `[ ]` | `get_related_products(products, item_id, limit)` |
| Add Python binding smoke test | `[ ]` | Import module and call each function |

Phase 6 Done When:

- [ ] Python can import the compiled C++ module.
- [ ] All exposed functions return testable values.
- [ ] Frontend remains unchanged.

## Phase 7: Use C++ Inside FastAPI

Goal: keep the frontend API unchanged while FastAPI uses C++ internally for ADS
logic.

| Task | Status | Evidence / Notes |
|---|---|---|
| Load clean DB data in Python services | `[ ]` | Python services currently use fallback logic |
| Pass ranking inputs to C++ | `[ ]` | Start after `biteapple_core` exists |
| Use C++ for recommendations | `[ ]` | Future `GET /recommendations/{user_id}` integration |
| Use C++ for related products | `[ ]` | Future `GET /products/{item_id}/related` integration |
| Use C++ for recent/ranking logic where appropriate | `[ ]` | Future recent/trending integration |
| Preserve frontend response shapes | `[ ]` | Must remain unchanged during integration |

Phase 7 Done When:

- [ ] Recommendations route uses C++.
- [ ] Related products route uses C++.
- [ ] Interaction/ranking behavior remains stable.
- [ ] API contract still matches frontend expectations.

## Phase 8: Final Testing And Demo Polish

Goal: make the system demo-ready and make the ADS data structures visible.

| Task | Status | Evidence / Notes |
|---|---|---|
| Frontend build passes | `[ ]` | Final run |
| Backend runs and `/docs` opens | `[ ]` | Final run |
| Database connects and has seed data | `[ ]` | Final run |
| Paymob payment sandbox flow works | `[ ]` | Checkout creates payment attempt and status |
| C++ builds | `[ ]` | Final run |
| pybind module imports | `[ ]` | Final run |
| Full user flow works | `[ ]` | signup, login, browse, cart, checkout |
| Payment flow works | `[ ]` | checkout, payment status, success/failure handling |
| Dashboard flow works | `[ ]` | add/edit/delete product |
| Recommendation flow works | `[ ]` | recommendations/related update or appear |
| Create final demo guide | `[ ]` | How to run and present the project |

Phase 8 Done When:

- [ ] One command list can start everything.
- [ ] Demo scenario is documented step by step.
- [ ] ADS structures and their roles are clear to the grader.

## Quick Commands

Frontend:

```powershell
cd frontend
npm.cmd install
npm.cmd run dev
npm.cmd run dev:admin
npm.cmd run build
npm.cmd run build:admin
```

Future backend:

```powershell
cd backend
python -m venv venv
.\venv\Scripts\activate
pip install -r requirements.txt
uvicorn app.main:app --reload
```

Repo hygiene:

```powershell
git status --short
rg "fetch\(|axios|XMLHttpRequest" frontend/src
```

Fawry mock checkout:

```powershell
cd backend
python -m app.seed
python -m uvicorn app.main:app --reload
```

Then use the cart checkout in the frontend, or call:

```powershell
Invoke-RestMethod -Method Post http://127.0.0.1:8000/users/1/checkout
Invoke-RestMethod http://127.0.0.1:8000/orders/ORDER_ID/payment
Invoke-RestMethod -Method Post http://127.0.0.1:8000/payments/orders/ORDER_ID/mock-paid
```

## Session Notes

Add one line after each work session:

| Date | Phase | What Changed | Verified | Next |
|---|---|---|---|---|
| 2026-05-07 | 0 | Frontend audit completed; dashboard separated; ignore cleanup started | build/dev/routes/auth checked | Stabilize current source status, then lock API naming |
| 2026-05-07 | 2 | Dummy FastAPI backend added with in-memory services and route aliases | Swagger and endpoint smoke test passed | Phase 3: switch `api.js` internals to fetch |
| 2026-05-07 | 3 | Frontend service layer switched from mock state to FastAPI fetch calls | user/admin builds, direct fetch search, browser smoke test | Phase 4: PostgreSQL persistence |
| 2026-05-07 | 4 | SQLAlchemy database layer added with users, items, interactions, cart, orders, payment attempts, seed data, and password hashing | seed, backend smoke test, persistence after restart, frontend/admin builds | Configure real PostgreSQL URL, then Phase 4.5 Fawry |
| 2026-05-07 | 4.5 | Fawry reference-payment flow added with order snapshots, payment attempts, status endpoints, and mock-paid demo path | backend mock checkout/status/paid routes, user/admin frontend builds | Add real Fawry sandbox credentials on target machine |
| 2026-05-07 | 5-7 | Removed backup C++ core from this version and added the future C++ function contract | Python fallback services ready for C++ team handoff | C++ team implements `docs/cpp-core-function-contract.md` |
