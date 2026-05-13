#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_DIR="$ROOT_DIR/backend"
FRONTEND_DIR="$ROOT_DIR/frontend"

PYTHON_BIN="${PYTHON_BIN:-python3}"
NODE_BIN="${NODE_BIN:-node}"
NPM_BIN="${NPM_BIN:-npm}"

BACKEND_HOST="${BACKEND_HOST:-127.0.0.1}"
BACKEND_PORT="${BACKEND_PORT:-8000}"
FRONTEND_HOST="${FRONTEND_HOST:-127.0.0.1}"
FRONTEND_PORT="${FRONTEND_PORT:-5173}"

export BITEAPPLE_CORE_PATH="${BITEAPPLE_CORE_PATH:-$ROOT_DIR/build}"
export JWT_SECRET="${JWT_SECRET:-dev-change-me}"
export JWT_ACCESS_EXPIRE_MINUTES="${JWT_ACCESS_EXPIRE_MINUTES:-60}"
export VITE_API_BASE_URL="${VITE_API_BASE_URL:-/api}"

if ! command -v "$PYTHON_BIN" >/dev/null 2>&1; then
  echo "Python not found: $PYTHON_BIN" >&2
  exit 1
fi

if ! command -v "$NODE_BIN" >/dev/null 2>&1; then
  echo "Node.js not found: $NODE_BIN" >&2
  exit 1
fi

if ! command -v "$NPM_BIN" >/dev/null 2>&1; then
  echo "npm not found: $NPM_BIN" >&2
  exit 1
fi

echo "Installing backend dependencies..."
"$PYTHON_BIN" -m pip install -r "$BACKEND_DIR/requirements.txt"

echo "Installing frontend dependencies..."
(
  cd "$FRONTEND_DIR"
  "$NPM_BIN" ci
)

echo "Building frontend..."
(
  cd "$FRONTEND_DIR"
  "$NPM_BIN" run build
)

cleanup() {
  if [[ -n "${BACKEND_PID:-}" ]] && kill -0 "$BACKEND_PID" >/dev/null 2>&1; then
    kill "$BACKEND_PID" >/dev/null 2>&1 || true
  fi
  if [[ -n "${FRONTEND_PID:-}" ]] && kill -0 "$FRONTEND_PID" >/dev/null 2>&1; then
    kill "$FRONTEND_PID" >/dev/null 2>&1 || true
  fi
}

trap cleanup EXIT INT TERM

echo "Starting backend on http://$BACKEND_HOST:$BACKEND_PORT ..."
(
  cd "$BACKEND_DIR"
  BACKEND_HOST="$BACKEND_HOST" BACKEND_PORT="$BACKEND_PORT" \
    "$PYTHON_BIN" -m uvicorn app.main:app --host "$BACKEND_HOST" --port "$BACKEND_PORT"
) &
BACKEND_PID=$!

echo "Waiting for backend to become ready..."
for attempt in {1..30}; do
  if "$PYTHON_BIN" - <<PY >/dev/null 2>&1; then
import urllib.request
urllib.request.urlopen('http://$BACKEND_HOST:$BACKEND_PORT/', timeout=2)
PY
    break
  fi
  sleep 1
done

echo "Starting frontend on http://$FRONTEND_HOST:$FRONTEND_PORT ..."
(
  cd "$FRONTEND_DIR"
  VITE_API_BASE_URL="$VITE_API_BASE_URL" "$NPM_BIN" run dev -- --host "$FRONTEND_HOST" --port "$FRONTEND_PORT"
) &
FRONTEND_PID=$!

echo
echo "BiteApple is running:"
echo "  Backend : http://$BACKEND_HOST:$BACKEND_PORT"
echo "  Frontend: http://$FRONTEND_HOST:$FRONTEND_PORT"
echo
echo "Press Ctrl+C to stop both servers."

wait