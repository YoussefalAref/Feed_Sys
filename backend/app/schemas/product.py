from pydantic import BaseModel


class ProductInput(BaseModel):
    name: str
    price: float
    category: str
    stock: int = 0
    image: str = ""
    description: str = ""
    popularity_score: float = 40
