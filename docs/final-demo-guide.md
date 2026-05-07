# BiteApple Final Demo Guide

Use this guide for the stage 8 presentation and the final push-ready demo.

## What To Show

1. Open the frontend and sign up or log in with a seeded account.
2. Browse products and explain that recommendations are produced by the C++ core.
3. Add an item to the cart and start checkout.
4. Show the payment status flow and explain that Paymob uses mock mode locally unless real sandbox credentials are set.
5. Open the admin dashboard and demonstrate add, edit, and delete product actions.
6. Point out that the recommendation and related-product sections update after interactions.

## How To Start The System

Open three terminals from the repo root.

Backend:

```powershell
cd backend
python -m app.seed
python -m uvicorn app.main:app --reload
```

Frontend:

```powershell
cd frontend
npm.cmd install
npm.cmd run dev
```

Admin dashboard:

```powershell
cd frontend
npm.cmd run dev:admin
```

If you need the C++/pybind demo build first:

```powershell
cmake -S . -B build\phase5
cmake --build build\phase5
cmake -S . -B build\phase6
cmake --build build\phase6
python backend\scripts\smoke_pybind.py
```

## Expected URLs

- User app: `http://127.0.0.1:5173/`
- Admin app: `http://127.0.0.1:5174/`
- API docs: `http://127.0.0.1:8000/docs`

## Key Grader Notes

- The recommendation engine runs through the C++ core exposed to Python.
- The payment flow is Paymob-oriented and supports local mock checkout.
- The database can run on PostgreSQL in the target setup, with SQLite as the local fallback.
- The main data structures to mention are the hash map, queue, max heap, and graph.