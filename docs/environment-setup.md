# BiteApple Environment Setup

Use this guide when moving the project to a new laptop, lab machine, or demo
machine.

## Current Machine Status

This machine does not have PostgreSQL configured for BiteApple.

For Phase 4 verification, the backend used the SQLite fallback:

```text
backend/biteapple_dev.db
```

That local `.db` file is ignored by Git and should not be treated as the final
project database. The real target for Phase 4 is PostgreSQL through
`DATABASE_URL`.

## Required Tools On The Target Machine

Install these before running the full project:

- Git
- Python 3.11 or newer
- Node.js and npm
- PostgreSQL

Optional for Phase 4.5 and later:

- Paymob sandbox credentials
- C++ compiler, CMake, and pybind11 for the separate C++ team integration later

## Backend Setup

From the repo root:

```powershell
cd backend
python -m venv .venv
.\.venv\Scripts\activate
pip install -r requirements.txt
```

Create a real local env file:

```powershell
Copy-Item .env.example .env
```

Edit `backend/.env` and set:

```text
DATABASE_URL=postgresql+psycopg2://postgres:YOUR_PASSWORD@localhost:5432/biteapple
```

Important Paymob step (required for Phase 4.5 sandbox/live testing):

- Copy the Paymob placeholder keys from `backend/.env.example` into `backend/.env`.
- Set `PAYMOB_MOCK_MODE=false` and fill the following required fields from your
  Paymob sandbox dashboard: `PAYMOB_API_KEY`, `PAYMOB_INTEGRATION_ID`,
  `PAYMOB_PUBLIC_KEY`, `PAYMOB_HMAC_SECRET`, and `PAYMOB_WEBHOOK_URL`.
- When testing locally with `ngrok`, start `ngrok` and update the Paymob
  integration's Callback/Webhook URL to your current ngrok URL, for example:

```
https://<your-ngrok-id>.ngrok.io/paymob/webhook
```

Note: ngrok URLs change on each run; update the Paymob integration webhook each
time you restart ngrok.

Do not commit `backend/.env`.

## PostgreSQL Setup On The Target Machine

1. Install PostgreSQL.
2. Start the PostgreSQL service.
3. Create the database:

```powershell
createdb -U postgres biteapple
```

If `createdb` is not in PATH, use `psql`:

```sql
CREATE DATABASE biteapple;
```

4. Confirm the app can connect:

```powershell
cd backend
.\.venv\Scripts\activate
python -m app.seed
python -m uvicorn app.main:app --reload
```

5. Open:

```text
http://127.0.0.1:8000/docs
```

If seeding succeeds and Swagger opens, PostgreSQL is connected.

## Frontend Setup

In another terminal:

```powershell
cd frontend
npm.cmd install
npm.cmd run dev
```

Open:

```text
http://127.0.0.1:5173/
```

Admin dashboard:

```powershell
cd frontend
npm.cmd run dev:admin
```

Open:

```text
http://127.0.0.1:5174/
```

## Expected Running URLs

| Layer | URL |
|---|---|
| FastAPI backend | `http://127.0.0.1:8000/` |
| FastAPI Swagger | `http://127.0.0.1:8000/docs` |
| User frontend | `http://127.0.0.1:5173/` |
| Admin dashboard | `http://127.0.0.1:5174/` |

## Verification Commands

Backend:

```powershell
cd backend
python -m app.seed
python -m uvicorn app.main:app --reload
```

Frontend:

```powershell
cd frontend
npm.cmd run build
npm.cmd run build:admin
```

API check:

```powershell
Invoke-WebRequest -UseBasicParsing http://127.0.0.1:8000/products
Invoke-WebRequest -UseBasicParsing http://127.0.0.1:8000/manager/dashboard
```

Frontend API boundary check:

```powershell
rg "fetch\(|axios|XMLHttpRequest" frontend/src
```

Expected result: direct HTTP calls should only appear in
`frontend/src/services/api.js`.

## Environment Variables To Revisit Later

When phases change, update this section.

### Phase 4: Database

```text
DATABASE_URL=postgresql+psycopg2://postgres:YOUR_PASSWORD@localhost:5432/biteapple
```

### Phase 4.5: Paymob

Mock mode works without real credentials:

```text
PAYMOB_MOCK_MODE=true
PAYMOB_API_KEY=
PAYMOB_INTEGRATION_ID=
PAYMOB_PUBLIC_KEY=
PAYMOB_HMAC_SECRET=
PAYMOB_CHECKOUT_URL=https://accept.paymob.com/standalone
PAYMOB_CURRENCY=EGP
PAYMOB_WEBHOOK_URL=
```

For real sandbox testing, set `PAYMOB_MOCK_MODE=false`, fill the Paymob
credentials from the sandbox dashboard, and keep those secrets only in
`backend/.env`.

Local FastAPI payment endpoints:

```text
POST /users/{user_id}/checkout
GET /orders/{order_id}/payment
POST /payments/orders/{order_id}/mock-paid
```

### Future C++ / pybind

The C++ core is intentionally not included in this version. The C++ team should
build against `docs/cpp-core-function-contract.md`, then add the pybind module
after the backend/frontend API is stable.

## Troubleshooting

### PostgreSQL Is Not Installed

The app will still run with SQLite if `DATABASE_URL` is missing. That is useful
for local frontend/backend testing, but it is not the final demo setup.

### `python -m app.seed` Fails To Connect

Check:

- PostgreSQL service is running.
- Database `biteapple` exists.
- Username/password in `backend/.env` are correct.
- `DATABASE_URL` starts with `postgresql+psycopg2://`.

### PowerShell Blocks `npm`

Use `npm.cmd` instead of `npm`:

```powershell
npm.cmd install
npm.cmd run dev
```

### Data Looks Old

If using SQLite fallback, delete the ignored file:

```powershell
Remove-Item backend\biteapple_dev.db
cd backend
python -m app.seed
```

For PostgreSQL, reset the database only if you intentionally want fresh seed
data.
