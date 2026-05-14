#!/usr/bin/env bash
set -euo pipefail

# Color output for better visibility
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[$(date +'%H:%M:%S')]${NC} $1"
}

print_error() {
    echo -e "${RED}[$(date +'%H:%M:%S')] ERROR:${NC} $1" >&2
}

print_warning() {
    echo -e "${YELLOW}[$(date +'%H:%M:%S')] WARNING:${NC} $1"
}

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_DIR="$ROOT_DIR/backend"
FRONTEND_DIR="$ROOT_DIR/frontend"
BUILD_DIR="$ROOT_DIR/build"

PYTHON_BIN="${PYTHON_BIN:-python3}"
NODE_BIN="${NODE_BIN:-node}"
NPM_BIN="${NPM_BIN:-npm}"
CMAKE_BIN="${CMAKE_BIN:-cmake}"

BACKEND_HOST="${BACKEND_HOST:-127.0.0.1}"
BACKEND_PORT="${BACKEND_PORT:-8000}"
FRONTEND_HOST="${FRONTEND_HOST:-127.0.0.1}"
FRONTEND_PORT="${FRONTEND_PORT:-5173}"

export BITEAPPLE_CORE_PATH="${BITEAPPLE_CORE_PATH:-$BUILD_DIR}"
export JWT_SECRET="${JWT_SECRET:-dev-change-me}"
export JWT_ACCESS_EXPIRE_MINUTES="${JWT_ACCESS_EXPIRE_MINUTES:-60}"
export VITE_API_BASE_URL="${VITE_API_BASE_URL:-/api}"

# Trap to cleanup on exit
cleanup() {
    print_status "Cleaning up..."
    if [[ -n "${BACKEND_PID:-}" ]] && kill -0 "$BACKEND_PID" >/dev/null 2>&1; then
        print_status "Stopping backend (PID: $BACKEND_PID)"
        kill "$BACKEND_PID" >/dev/null 2>&1 || true
    fi
    if [[ -n "${FRONTEND_PID:-}" ]] && kill -0 "$FRONTEND_PID" >/dev/null 2>&1; then
        print_status "Stopping frontend (PID: $FRONTEND_PID)"
        kill "$FRONTEND_PID" >/dev/null 2>&1 || true
    fi
}

trap cleanup EXIT INT TERM

# Check required commands
print_status "Checking for required tools..."

if ! command -v "$PYTHON_BIN" >/dev/null 2>&1; then
    print_error "Python not found: $PYTHON_BIN"
    exit 1
fi
print_status "✓ Python found: $("$PYTHON_BIN" --version)"

if ! command -v "$NODE_BIN" >/dev/null 2>&1; then
    print_error "Node.js not found: $NODE_BIN"
    exit 1
fi
print_status "✓ Node.js found: $("$NODE_BIN" --version)"

if ! command -v "$NPM_BIN" >/dev/null 2>&1; then
    print_error "npm not found: $NPM_BIN"
    exit 1
fi
print_status "✓ npm found: $("$NPM_BIN" --version)"

if ! command -v "$CMAKE_BIN" >/dev/null 2>&1; then
    print_warning "CMake not found: $CMAKE_BIN (C++ build will be skipped)"
    CMAKE_AVAILABLE=false
else
    print_status "✓ CMake found: $("$CMAKE_BIN" --version | head -1)"
    CMAKE_AVAILABLE=true
fi

# Build C++ project if CMake is available and binary doesn't exist or is outdated
if [[ "$CMAKE_AVAILABLE" == "true" ]]; then
    print_status "Building C++ core..."

    # Check if build directory exists and if sources are newer than binary
    NEEDS_BUILD=false
    CPP_BINARY=""
    for f in "$BUILD_DIR"/biteapple_core*.so; do
        [[ -f "$f" ]] && CPP_BINARY="$f" && break
    done

    if [[ -z "$CPP_BINARY" ]]; then
        print_warning "C++ binary not found, building..."
        NEEDS_BUILD=true
    else
        # Enable globstar so ** recurses into subdirectories (e.g. src/bindings/)
        shopt -s globstar 2>/dev/null || true
        for src in "$ROOT_DIR"/src/**/*.{cpp,h} "$ROOT_DIR"/CMakeLists.txt; do
            [[ -f "$src" ]] && [[ "$src" -nt "$CPP_BINARY" ]] && {
                print_warning "Source files updated, rebuilding..."
                NEEDS_BUILD=true
                break
            }
        done
        shopt -u globstar 2>/dev/null || true
    fi
    
    if [[ "$NEEDS_BUILD" == "true" ]]; then
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        
        if ! "$CMAKE_BIN" .. -DCMAKE_BUILD_TYPE=Release; then
            print_error "CMake configuration failed"
            exit 1
        fi
        
        if ! "$CMAKE_BIN" --build . --config Release; then
            print_error "C++ build failed"
            exit 1
        fi
        
        print_status "✓ C++ core built successfully"
        cd "$ROOT_DIR"
    else
        print_status "✓ C++ core is up to date, skipping build"
    fi
else
    if [[ -f "$BUILD_DIR/biteapple_core"*.so ]]; then
        print_status "✓ Using existing C++ binary (CMake not available)"
    else
        print_warning "C++ binary not found and CMake unavailable - some features may not work"
    fi
fi

# Install Python backend dependencies
print_status "Installing backend dependencies..."
if ! "$PYTHON_BIN" -m pip install -q -r "$BACKEND_DIR/requirements.txt"; then
    print_error "Failed to install backend dependencies"
    exit 1
fi
print_status "✓ Backend dependencies installed"

# Install frontend dependencies
print_status "Installing frontend dependencies..."
if ! (cd "$FRONTEND_DIR" && "$NPM_BIN" ci --silent); then
    print_error "Failed to install frontend dependencies"
    exit 1
fi
print_status "✓ Frontend dependencies installed"

# Build frontend
print_status "Building frontend..."
if ! (cd "$FRONTEND_DIR" && "$NPM_BIN" run build); then
    print_error "Frontend build failed"
    exit 1
fi
print_status "✓ Frontend built successfully"

# Start backend
print_status "Starting backend on http://$BACKEND_HOST:$BACKEND_PORT..."
(
    cd "$BACKEND_DIR"
    BACKEND_HOST="$BACKEND_HOST" BACKEND_PORT="$BACKEND_PORT" \
        "$PYTHON_BIN" -m uvicorn app.main:app --host "$BACKEND_HOST" --port "$BACKEND_PORT" --reload
) &
BACKEND_PID=$!
print_status "Backend started with PID: $BACKEND_PID"

# Wait for backend to be ready
print_status "Waiting for backend to become ready..."
for attempt in {1..30}; do
    if "$PYTHON_BIN" -c "import urllib.request; urllib.request.urlopen('http://$BACKEND_HOST:$BACKEND_PORT/', timeout=2)" >/dev/null 2>&1; then
        print_status "✓ Backend is ready!"
        break
    fi
    
    if ! kill -0 "$BACKEND_PID" >/dev/null 2>&1; then
        print_error "Backend process died. Check logs above."
        exit 1
    fi
    
    if [[ $attempt -eq 30 ]]; then
        print_error "Backend failed to become ready after 30 seconds"
        exit 1
    fi
    
    sleep 1
done

# Start frontend
print_status "Starting frontend on http://$FRONTEND_HOST:$FRONTEND_PORT..."
(
    cd "$FRONTEND_DIR"
    VITE_API_BASE_URL="$VITE_API_BASE_URL" "$NPM_BIN" run dev -- --host "$FRONTEND_HOST" --port "$FRONTEND_PORT"
) &
FRONTEND_PID=$!
print_status "Frontend started with PID: $FRONTEND_PID"

# Display running information
echo ""
echo "============================================"
print_status "BiteApple is running!"
echo "============================================"
echo "  Backend  : http://$BACKEND_HOST:$BACKEND_PORT"
echo "  API Docs : http://$BACKEND_HOST:$BACKEND_PORT/docs"
echo "  Frontend : http://$FRONTEND_HOST:$FRONTEND_PORT"
echo "============================================"
echo ""
print_status "Press Ctrl+C to stop all services"
echo ""

# Wait for any process to finish
wait
