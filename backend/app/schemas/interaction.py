from pydantic import BaseModel


class InteractionInput(BaseModel):
    user_id: int
    item_id: int
    type: str
