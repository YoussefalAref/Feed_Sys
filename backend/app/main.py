from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from app.database import init_db
from app.routes import auth, cart, dashboard, interactions, payments, products, recommendations

app = FastAPI(
    title="BiteApple API",
    description="FastAPI backend for BiteApple frontend integration.",
    version="0.1.0",
)

allowed_origins = [
    "http://localhost:5173",
    "http://127.0.0.1:5173",
    "http://localhost:5174",
    "http://127.0.0.1:5174",
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=allowed_origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(auth.router)
app.include_router(products.router)
app.include_router(interactions.router)
app.include_router(recommendations.router)
app.include_router(cart.router)
app.include_router(payments.router)
app.include_router(dashboard.router)


@app.get("/")
def health_check() -> dict:
    return {
        "name": "BiteApple Dummy API",
        "status": "ok",
        "phase": "4.5",
    }


@app.on_event("startup")
def on_startup() -> None:
    init_db()
