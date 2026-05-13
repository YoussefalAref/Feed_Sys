# Feed_Sys / BiteApple Project Guide A to Z

This document explains the project end to end: what it is, how it is structured, how data flows through it, how to build and run it locally, and what each major file and subsystem does.

## A. What The Project Is

Feed_Sys, also referred to in the UI and docs as BiteApple, is a smart e-commerce recommendation platform. It combines:

- A React frontend for the user experience.
- A FastAPI backend for HTTP routes, validation, and orchestration.
- A C++ core exposed through `pybind11` for the recommendation and data-structure logic.
- Python service wrappers that either call the C++ core or fall back to SQLAlchemy-backed database logic.

The goal is to simulate an e-commerce feed that reacts to user interactions such as views, clicks, cart additions, and purchases.

## B. Core Idea

The application is built around one main idea: user behavior should influence what products are shown next.

If a product is popular, it should appear in trending feeds.
If a user interacts with certain categories, the system should surface related items.
If a user is returning, the app should personalize recommendations based on prior activity.

## C. Current Architecture

The current project uses a three-layer stack:

1. React frontend in `frontend/`
2. FastAPI backend in `backend/`
3. C++ core in `src/`

The frontend sends HTTP requests to the backend. The backend routes call Python service functions. Those services either:

- delegate to the C++ core through `backend/app/services/cpp_core.py`, or
- fall back to the Python database layer when needed.

The current C++ core is a working in-memory backup engine. That means it can satisfy the contract and power the app, but it does not persist state by itself.

## D. Main Technologies

- React 18
- Vite
- FastAPI
- SQLAlchemy
- SQLite by default for local Python persistence
- pybind11 for C++/Python bindings
- C++ custom data structures
- JWT for access tokens
- Argon2 for password hashing in Python

## E. Main Folders And Files

### Root level

- `README.md` - high-level project overview and architecture summary.
- `run-local.sh` - local launcher that installs dependencies, builds the frontend, and starts both servers.
- `schema.sql` - database schema.
- `PROJECT_ALIGNMENT_AND_IMPLEMENTATION_STATUS.md` - status and contract alignment notes.
- `PROJECT_A_TO_Z_GUIDE.md` - this document.

### Backend

- `backend/app/main.py` - FastAPI application setup and router registration.
- `backend/app/database.py` - SQLAlchemy engine, session factory, and database initialization.
- `backend/app/models.py` - ORM models.
- `backend/app/routes/` - HTTP endpoints.
- `backend/app/services/` - business logic, auth, serialization, and C++ bridge.
- `backend/app/deps/` - dependencies such as JWT auth helpers.
- `backend/app/seed.py` - seed data for local/demo usage.

### Frontend

- `frontend/src/App.jsx` - top-level app routes.
- `frontend/src/services/api.js` - browser API client.
- `frontend/src/pages/` - page components.
- `frontend/src/components/` - reusable UI components.

### C++

- `src/main.cpp` - C++ entry point for native build/demo flows.
- `src/bindings/biteapple_core.cpp` - pybind11 module exported to Python.
- `src/data_structures/` - graph, heap, queue, and hash map implementations.
- `src/models/` - C++ item, user, interaction, and recommendation-related models.

## F. Frontend Structure

The frontend is a Vite React app. The important pages are:

- Home feed
- Login
- Signup
- Cart
- Product details
- Dashboard

The main app routes are defined in `frontend/src/App.jsx`. The frontend fetches data from the backend through `frontend/src/services/api.js`.

The frontend now defaults to `/api` so the Vite dev server can proxy requests to the backend during local development.

## G. Backend Structure

The FastAPI backend exposes the HTTP API used by the frontend.

Key responsibilities:

- Validate request payloads.
- Convert request data into Python objects.
- Call the C++ core when available.
- Fall back to SQLAlchemy if needed.
- Return JSON responses to the frontend.

The backend is intentionally thin at the route layer. Most logic lives in service files under `backend/app/services/`.

## H. C++ Core Structure

The C++ core is built as a `pybind11` extension module named `biteapple_core`.

It currently exposes functions for:

- authentication
- user creation and lookup
- product CRUD
- cart operations
- checkout
- dashboard stats
- interaction tracking
- recommendations
- related products
- trending products

The core uses custom data structures such as:

- hash map
- queue
- heap
- graph

In the current backup implementation, the core is in-memory and seeded with demo data.

## I. Data Flow End To End

Here is the normal request path:

1. A user opens the frontend.
2. The frontend calls the backend API.
3. The backend route validates the request.
4. The backend service tries to call the C++ core.
5. If the C++ function exists and succeeds, the result is returned.
6. If the C++ function is unavailable or fails, the service falls back to the Python database implementation.
7. The response is serialized to JSON and returned to the browser.

Example:

- `GET /products` loads the product feed.
- `GET /recommendations/{user_id}` loads personalized recommendations.
- `POST /interactions` records user behavior.

## J. Authentication

Authentication uses two layers:

- Password hashing with Argon2 in Python.
- JWT issuance for tokens.

Relevant files:

- `backend/app/services/security.py`
- `backend/app/services/auth_jwt.py`
- `backend/app/services/auth_service.py`
- `backend/app/deps/jwt_dependency.py`

Current behavior:

- Login and signup return a token.
- The token is a JWT signed with the configured secret.
- Protected routes can use the JWT dependency to decode and validate the user identity.

Important note:

- The Python side uses secure password hashing.
- The in-memory C++ backup core still stores demo credentials in memory for the fallback implementation.

## K. Knowledge About Product Data

Products have fields such as:

- id
- name
- price
- category
- stock
- image
- description
- popularity score

The frontend displays these fields in the feed, details view, and recommendation sections.

## L. Local Development Setup

There are two common ways to run the project locally.

### Option 1: Use the launcher script

The root script `run-local.sh`:

- installs backend dependencies
- installs frontend dependencies
- builds the frontend
- starts backend and frontend servers

Run it with:

```bash
cd /workspaces/Feed_Sys
./run-local.sh
```

### Option 2: Start services manually

Backend:

```bash
cd backend
python -m uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
```

Frontend:

```bash
cd frontend
npm ci
npm run dev -- --host 0.0.0.0
```

## M. Build Process

### Backend dependencies

The backend dependencies are listed in `backend/requirements.txt`.

### Frontend build

The frontend build command is:

```bash
cd frontend
npm run build
```

### C++ build

The C++ extension is built with CMake and pybind11.

The compiled module is loaded by the backend through `backend/app/services/cpp_core.py`.

## N. Network And API URLs

Default local URLs:

- Frontend: `http://127.0.0.1:5173`
- Backend: `http://127.0.0.1:8000`
- Backend docs: `http://127.0.0.1:8000/docs`

During development, Vite proxies `/api` requests to the backend so the browser can talk to both services cleanly.

## O. Organization Of Routes

Main backend route groups:

- `auth` - login, signup, and user lookup
- `products` - product listing and CRUD
- `cart` - cart operations
- `interactions` - interaction logging and recent activity
- `recommendations` - recommendation endpoints
- `dashboard` - dashboard metrics
- `payments` - mock or provider-backed payment flow

## P. Python Service Layer

The service layer is the most important backend abstraction.

Examples:

- `auth_service.py` - login/signup and user lookup
- `product_service.py` - product fetch and CRUD
- `cart_service.py` - cart operations and checkout
- `interaction_service.py` - interaction recording and recent activity
- `recommendation_service.py` - trending, related products, and recommendations
- `dashboard_service.py` - aggregated stats
- `serialization.py` - converts ORM models to API-friendly dictionaries

Each service tries the C++ path first when available.

## Q. Query Examples

Frontend examples that hit the backend:

- `getProducts()` -> `GET /products`
- `getItem(id)` -> `GET /products/{id}`
- `getRecommendations(userId)` -> `GET /recommendations/{userId}`
- `getCart(userId)` -> `GET /users/{userId}/cart`

## R. Recommendations Logic

The recommendation system blends multiple signals:

- product popularity
- category similarity
- graph-based related products
- recent interactions
- user preference/category hints

Trending products are usually derived from popularity ranking.
Related products are derived from graph similarity or category fallback.
Personalized recommendations consider the user’s interaction history.

## S. State Management

There are two kinds of state in the project:

### Durable state

Handled by the Python database layer through SQLAlchemy and SQLite/PostgreSQL.

### Ephemeral state

Handled by the in-memory backup C++ core.

This means the Python DB path is the durable path, while the C++ backup core is useful for demos and contract completeness.

## T. Testing And Smoke Checks

Typical validation steps include:

- build the frontend
- start the backend
- call `/`, `/products`, and `/dashboard/stats`
- verify `/api/products` through the Vite proxy
- smoke-test JWT login/signup flows

The repo also includes a GitHub Actions workflow in `.github/workflows/ci.yml` that installs dependencies, builds both sides, and runs API smoke checks.

## U. UI Experience

The frontend includes:

- a hero section
- a product feed
- a recommendation section
- product cards
- cart interactions
- login and signup flows

The UI is designed to show live products and recommendation behavior, even when the current user is a guest.

## V. Version Control And CI

The repo now includes a GitHub Actions workflow that:

- installs backend dependencies
- builds the frontend
- starts the backend and frontend
- verifies key routes

Local development is handled by `run-local.sh`.

## W. Weaknesses / Current Limitations

The project is functional, but a few important things remain as future improvements:

- The backup C++ core is in-memory, not persistent.
- JWT refresh/revocation is not yet implemented.
- Some production hardening is still needed.
- Full automated test coverage is still limited.
- The payment flow remains a separate concern and depends on the target deployment setup.

## X. eXact Files To Know First

If you only read a handful of files to understand the system, start here:

1. `README.md`
2. `run-local.sh`
3. `backend/app/main.py`
4. `backend/app/services/auth_service.py`
5. `backend/app/services/cpp_core.py`
6. `backend/app/services/security.py`
7. `frontend/src/services/api.js`
8. `frontend/src/App.jsx`
9. `src/bindings/biteapple_core.cpp`

## Y. Why The Architecture Is Split This Way

The split exists for practical reasons:

- React is best for interactive UI.
- FastAPI is good for quick API orchestration and validation.
- Python is convenient for database logic and service glue.
- C++ is strong for the data-structure-heavy recommendation and ranking logic.

The separation also lets the project function even if only part of the C++ contract is available.

## Z. What To Tell A Reviewer Or Demo Panel

If you need to explain the project in a short presentation, say this:

> Feed_Sys is a smart e-commerce platform where React provides the UI, FastAPI exposes the backend, Python handles persistence and security, and C++ powers the recommendation logic through pybind11. The system tracks interactions, ranks products, returns personalized recommendations, and supports local development through a single launcher script.

## Local Run Summary

```bash
cd /workspaces/Feed_Sys
./run-local.sh
```

That is the fastest way to bring up the whole stack on a local machine.

## Final Notes

This guide describes the current state of the project as implemented in the repository. If you want, I can also create a shorter presentation version, a PDF-ready version, or a more formal architecture document for submission.