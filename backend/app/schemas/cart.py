from pydantic import BaseModel


class CartAddRequest(BaseModel):
    user_id: int | None = None
    item_id: int
    quantity: int = 1


class CartRemoveRequest(BaseModel):
    user_id: int
    item_id: int


class CartUpdateRequest(BaseModel):
    quantity: int
