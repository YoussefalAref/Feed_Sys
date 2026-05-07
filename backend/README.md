# BiteApple Dummy FastAPI Backend

This backend started as the Phase 2 dummy API and now has SQLAlchemy
persistence, pybind/C++ service integration, and a Paymob-ready checkout flow.

## Database

Machine setup notes live in `docs/environment-setup.md`.

Set `DATABASE_URL` in `backend/.env` for PostgreSQL:

```powershell
DATABASE_URL=postgresql+psycopg2://postgres:postgres@localhost:5432/biteapple
```

If `DATABASE_URL` is not set, the backend uses local SQLite at
`backend/biteapple_dev.db` so the app can run on machines without PostgreSQL.

## Run

```powershell
cd backend
pip install -r requirements.txt
python -m app.seed
python -m uvicorn app.main:app --reload
```

Open:

- API root: `http://127.0.0.1:8000/`
- Swagger docs: `http://127.0.0.1:8000/docs`

## Current Accounts

- `mariam@biteapple.test` / `password123`
- `omar@biteapple.test` / `password123`
- `manager@biteapple.test` / `manager123`

## Notes

- Response shapes mirror `frontend/src/services/api.js`.
- React should call this backend only through `frontend/src/services/api.js`.
- Passwords are hashed with PBKDF2 before storage.
- Checkout creates an order, stores an order-item snapshot, starts a Paymob
  payment attempt, and keeps the cart until payment is marked paid.
- Use `PAYMOB_MOCK_MODE=true` for local demo checkout without sandbox credentials.
