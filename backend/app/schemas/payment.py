from pydantic import BaseModel, Field


class PayRequest(BaseModel):
    amount: float = Field(..., gt=0, description="Amount in EGP")
    order_ref: str = Field(..., min_length=1, description="Unique order reference string")
    first_name: str = "Customer"
    last_name: str = "Customer"
    email: str = "customer@biteapple.test"
    phone: str = "+201234567890"
