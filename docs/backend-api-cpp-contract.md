# Backend API And C++ Binding Contract

This document maps the FastAPI routes to the future C++ core. It intentionally
keeps the same three-cluster plan from the team handoff:

1. Auth and Users
2. Products, Cart, and Dashboard
3. Interactions and Recommendations

For exact C++ DTOs, helper functions, data structures, and pybind names, use:

```text
docs/cpp-core-function-contract.md
```

## Architecture Rule

React calls FastAPI. FastAPI calls Python wrapper/service functions. Those
wrappers should call the C++ core through pybind.

```text
React
  -> FastAPI route
  -> Python wrapper/service
  -> pybind module: biteapple_core
  -> C++ custom data structures
```

Python should stay thin:

- no recommendation scoring in Python
- no interaction queue in Python
- no heap/ranking logic in Python
- no graph similarity logic in Python
- no cart business logic in Python after the core is integrated

Python may still handle:

- HTTP request/response validation
- JSON conversion
- payment provider calls/secrets/webhooks
- optional DB persistence/snapshot loading

## Cluster 1: Auth And Users

| Frontend function | FastAPI endpoint | Python wrapper should call |
|---|---|---|
| `login(email, password)` | `POST /auth/login` | `biteapple_core.authenticate_user(email, password)` |
| `signup(userData)` | `POST /auth/signup` | `biteapple_core.create_user(input)` |
| internal/current user | `GET /users/{user_id}` | `biteapple_core.get_user_by_id(user_id)` |

Required C++ structures:

- `HashMap<int, User>` for user lookup.
- Optional `HashMap<std::string, int>` for email lookup.

## Cluster 2: Products, Cart, And Dashboard

| Frontend function | FastAPI endpoint | Python wrapper should call |
|---|---|---|
| `getProducts()` | `GET /products` | `biteapple_core.list_products(category)` |
| `getItemsByCategory(category)` | `GET /products/category/{category}` | `biteapple_core.list_products(category)` |
| `getItem(itemId)` | `GET /products/{item_id}` | `biteapple_core.get_product_by_id(item_id)` |
| `addProduct(productData)` | `POST /products` | `biteapple_core.create_product(input)` |
| `updateProduct(itemId, productData)` | `PUT /products/{item_id}` | `biteapple_core.update_product(item_id, input)` |
| `deleteProduct(itemId)` | `DELETE /products/{item_id}` | `biteapple_core.delete_product(item_id)` |
| `getCart(userId)` | `GET /users/{user_id}/cart` | `biteapple_core.get_cart(user_id)` |
| `addToCart(userId, itemId, quantity)` | `POST /users/{user_id}/cart` | `biteapple_core.add_to_cart(user_id, item_id, quantity)` |
| `removeFromCart(userId, itemId)` | `DELETE /users/{user_id}/cart/{item_id}` | `biteapple_core.remove_from_cart(user_id, item_id)` |
| `checkout(userId)` | `POST /users/{user_id}/checkout` | `biteapple_core.create_checkout_draft(user_id)` |
| payment finalize | internal after paid payment | `biteapple_core.finalize_paid_checkout(user_id)` |
| `getDashboardStats()` | `GET /dashboard/stats` | `biteapple_core.get_dashboard_stats()` |

Required C++ structures:

- `HashMap<int, Item>` for product storage.
- `HashMap<int, Cart>` or equivalent custom cart map.
- `MaxHeap` for trending/ranking.
- `HashMap` category aggregation for dashboard stats.

Payment note:

- Python calls Paymob/Fawry or mock payment endpoints.
- C++ creates the checkout draft and finalizes paid checkout.
- Payment provider secrets and webhooks stay out of C++.

## Cluster 3: Interactions And Recommendations

| Frontend function | FastAPI endpoint | Python wrapper should call |
|---|---|---|
| `recordInteraction(userId, itemId, type)` | `POST /interactions` | `biteapple_core.record_interaction(user_id, item_id, type)` |
| `getRecentInteractions()` | `GET /interactions/recent` | `biteapple_core.get_recent_interactions(limit)` |
| `getRecommendations(userId)` | `GET /recommendations/{user_id}` | `biteapple_core.get_recommendations(user_id, limit)` |
| `getRelatedProducts(itemId)` | `GET /products/{item_id}/related` | `biteapple_core.get_related_products(item_id, limit)` |
| home/trending feed | `GET /trending` | `biteapple_core.get_trending(limit)` |

Required C++ structures:

- `Queue<Interaction>` must store the recent interaction stream.
- `HashMap<int, User>` updates user activity score and level.
- `HashMap<int, Item>` updates product popularity.
- `MaxHeap` reflects changed product scores.
- `Graph` returns related item candidates.

## Required pybind Module

Module name:

```text
biteapple_core
```

Required exported functions:

```text
reset_core()
load_users(users)
load_products(products)
load_interactions(interactions)
load_cart(cart_items)
is_core_ready()

authenticate_user(email, password)
create_user(input)
get_user_by_id(user_id)

list_products(category)
get_product_by_id(item_id)
create_product(input)
update_product(item_id, input)
delete_product(item_id)

get_cart(user_id)
add_to_cart(user_id, item_id, quantity)
remove_from_cart(user_id, item_id)
create_checkout_draft(user_id)
finalize_paid_checkout(user_id)

record_interaction(user_id, item_id, interaction_type)
get_recent_interactions(limit)
get_recommendations(user_id, limit)
get_related_products(item_id, limit)
get_trending(limit)
get_dashboard_stats()
```

## Python Wrapper Pattern Later

Every service should eventually become thin:

```python
from app.services.core_loader import core

def get_recommendations(user_id: int, limit: int = 4) -> list[dict]:
    return to_json(core.get_recommendations(user_id, limit))
```

The backend should keep fallback Python logic only until the C++ core is ready.

## Do Not Change

- Existing frontend service function names.
- Existing FastAPI route paths.
- Existing JSON response field names.
- The pybind function names in this contract.
