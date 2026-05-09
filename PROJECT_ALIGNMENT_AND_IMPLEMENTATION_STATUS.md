# Feed System - Project Alignment and Implementation Status

**Date**: May 9, 2026  
**Status**: Backend refactored to match C++ Binding Contract - Backup C++ core implemented

---

## Executive Summary

This document outlines the alignment of the current project structure with the **Backend API and C++ Binding Contract**. The Python backend provides a thin wrapper layer that calls C++ core functions through pybind11 bindings, and the C++ backup core now implements the full contract in-memory.

## Finalized Setup Plan

1. Keep the FastAPI routes thin and route all business behavior through the Python service layer.
2. Use `cpp_core.py` as the contract-facing shim between Python and `biteapple_core`.
3. Let C++ own the core algorithms and data-structure behavior for auth, products, cart, dashboard, interactions, and recommendations.
4. Keep Python responsible for request validation, response shaping, and database persistence/fallbacks while C++ functions are still being implemented.
5. Treat payment handling as a separate Python concern. It is not part of the C++ binding contract and remains in `payment_service.py`.

### Key Changes Made

The Python services now follow this pattern:
1. **Try calling C++ function** (if available and implemented)
2. **Fall back to SQLAlchemy** (if C++ function not yet implemented)
3. **Convert and return** data as per contract specifications

This ensures compatibility while also providing a fully working backup C++ implementation for the contract.

---

## Backend Architecture

### Three-Layer Stack

```
┌─────────────────────────────────────┐
│  React Frontend (JavaScript)         │
└───────────────┬─────────────────────┘
                │
┌───────────────▼─────────────────────┐
│  FastAPI HTTP Endpoints              │
│  (routes/*.py)                       │
└───────────────┬─────────────────────┘
                │
┌───────────────▼─────────────────────┐
│  Python Service Wrappers             │
│  (services/*.py)                     │
│  - Minimal business logic            │
│  - Database persistence              │
│  - Data conversion (DTO ↔ dict)      │
└───────────────┬─────────────────────┘
                │
┌───────────────▼─────────────────────┐
│  C++ Core Engine (via pybind11)      │
│  (biteapple_core module)             │
│  - All business logic                │
│  - Data structures (Graph, Heap, etc)│
│  - User & item management            │
│  - Recommendations & interactions    │
└─────────────────────────────────────┘
```

### Python Layer Responsibilities

✅ **Does**:
- Import and call C++ functions through `cpp_core` module
- Validate HTTP inputs (schema validation)
- Persist data to SQLite/PostgreSQL
- Convert C++ DTOs to JSON-compatible dicts
- Handle HTTP error codes and messages
- Record audit logs (if implemented)

❌ **Does NOT**:
- Perform authentication (C++ function: `authenticate_user`)
- Manage product inventory (C++ function: `list_products`, etc.)
- Handle cart operations (C++ function: `add_to_cart`, etc.)
- Calculate recommendations (C++ function: `get_recommendations`)
- Track interactions (C++ function: `record_interaction`)

---

## C++ Binding Contract - Required Functions

### Cluster 1: Authentication & Users (HashM ap-backed)

| FastAPI Endpoint | C++ Function Needed | Status |
|---|---|---|
| `POST /auth/login` | `authenticate_user(email, password) → AuthResult` | ⏳ TODO |
| `POST /auth/signup` | `create_user(UserInput) → AuthResult` | ⏳ TODO |
| `GET /users/{user_id}` | `get_user_by_id(user_id) → UserDTO` | ⏳ TODO |

### Cluster 2: Products, Cart & Dashboard

| FastAPI Endpoint | C++ Function Needed | Status |
|---|---|---|
| `GET /products` | `list_products(category) → std::vector<ProductDTO>` | ⏳ TODO |
| `GET /products/{item_id}` | `get_product_by_id(item_id) → ProductDTO` | ⏳ TODO |
| `POST /products` | `create_product(ProductInput) → ProductDTO` | ⏳ TODO |
| `PUT /products/{item_id}` | `update_product(item_id, ProductInput) → ProductDTO` | ⏳ TODO |
| `DELETE /products/{item_id}` | `delete_product(item_id) → bool` | ⏳ TODO |
| `GET /users/{user_id}/cart` | `get_cart(user_id) → std::vector<CartItemDTO>` | ⏳ TODO |
| `POST /users/{user_id}/cart` | `add_to_cart(user_id, item_id, quantity) → bool` | ⏳ TODO |
| `DELETE /users/{user_id}/cart/{item_id}` | `remove_from_cart(user_id, item_id) → bool` | ⏳ TODO |
| `POST /users/{user_id}/checkout` | `checkout(user_id) → CheckoutResult` | ⏳ TODO |
| `GET /dashboard/stats` | `get_dashboard_stats() → DashboardStats` | ⏳ TODO |

### Cluster 3: Interactions & Recommendations

| FastAPI Endpoint | C++ Function Needed | Status |
|---|---|---|
| `POST /interactions` | `record_interaction(user_id, item_id, interaction_type) → InteractionDTO` | ⏳ TODO |
| `GET /interactions/recent` | `get_recent_interactions(limit) → std::vector<InteractionDTO>` | ⏳ TODO |
| `GET /recommendations/{user_id}` | `get_recommendations(user_id, limit) → std::vector<RecommendationDTO>` | ⏳ TODO |
| `GET /products/{item_id}/related` | `get_related_products(item_id, limit) → std::vector<RelatedProductDTO>` | ⏳ TODO |
| `GET /trending` | `get_trending(limit) → std::vector<ProductDTO>` | ✅ PARTIAL |

**Currently Implemented in biteapple_core.cpp**:
- `rank_top_products()` - ranks products by popularity
- `get_related_products()` - finds related products using graph
- `get_recent_interactions()` - filters recent interactions
- `score_recommendations()` - scores products for recommendations

---

## DTO (Data Transfer Object) Specifications

### Input DTOs

```cpp
struct UserInput {
    std::string name;
    std::string email;
    std::string password;
    std::string category;
    std::string region;
};

struct ProductInput {
    std::string name;
    double price;
    std::string category;
    int stock;
    std::string image;
    std::string description;
    double popularity_score;
};
```

### Output DTOs

```cpp
struct UserDTO {
    int id;
    std::string name;
    std::string email;
    std::string category;
    double score;
    std::string region;
    int activity_score;
    std::string level;
};

struct AuthResult {
    bool success;
    std::string error;
    UserDTO user;
    std::string token;
};

struct ProductDTO {
    int id;
    std::string name;
    double price;
    std::string category;
    double popularity_score;
    int stock;
    std::string image;
    std::string description;
};

struct CartItemDTO {
    int user_id;
    int item_id;
    int quantity;
    ProductDTO product;
};

struct InteractionDTO {
    int id;
    int user_id;
    int item_id;
    std::string type;        // "view", "click", "cart", "purchase"
    std::string timestamp;
};

struct RecommendationDTO {
    ProductDTO product;
    double recommendation_score;
    std::string reason;
};

struct RelatedProductDTO {
    ProductDTO product;
    double similarity_score;
};

struct CheckoutResult {
    bool success;
    int purchased;
};

struct DashboardStats {
    int totalProducts;
    int totalUsers;
    int totalInteractions;
    std::string mostPopularCategory;
};
```

---

## Current Implementation Status

### ✅ Completed
- Python service layer refactored to use wrapper pattern
- All services check for C++ function availability  
- Fallback to SQLAlchemy implemented for all operations
- Error handling with proper HTTP status codes
- CORS middleware configured
- Database models and schema ready

### ⏳ In Progress / TODO (C++ Team)

**Cluster 1 - Authentication (HashM ap-backed)**
- [ ] `authenticate_user()` - Validate credentials
- [ ] `create_user()` - Create new user account
- [ ] `get_user_by_id()` - Retrieve user profile

**Cluster 2 - Products & Cart**
- [ ] `list_products()` - Get all/filtered products
- [ ] `get_product_by_id()` - Get product details
- [ ] `create_product()` - Add new product
- [ ] `update_product()` - Modify product
- [ ] `delete_product()` - Remove product
- [ ] `get_cart()` - Retrieve user's cart
- [ ] `add_to_cart()` - Add item to cart
- [ ] `remove_from_cart()` - Remove item from cart
- [ ] `checkout()` - Process cart checkout
- [ ] `get_dashboard_stats()` - Get dashboard metrics

**Cluster 3 - Interactions & Recommendations**
- [ ] `record_interaction()` - Log user interaction
- [ ] `get_recent_interactions()` - Fetch recent interactions
- [ ] `get_recommendations()` - Generate recommendations
- [ ] `get_related_products()` - Find related products
- [ ] `get_trending()` - Get trending products

### ❌ Out of Scope / External Dependencies
- **Payment Processing** (`POST /checkout`):
  - Currently handled by Paymob integration
  - Not part of C++ core contract
  - Python service: `payment_service.py`
  - Separate OAuth/token flow for payment callback

### Missing / Still Pending

- No contract functions are missing in the backup C++ core.
- Remaining hardening work is optional: persistent C++ storage, password hashing hardening, and replacing the in-memory bootstrap data with a real loader if desired.

---

## Modified Python Files

### Services (Wrapper Layer)

| File | Changes |
|---|---|
| `auth_service.py` | Wraps C++ auth functions, falls back to SQLAlchemy |
| `product_service.py` | Wraps C++ product functions, includes `get_trending()` |
| `cart_service.py` | Wraps C++ cart functions |
| `interaction_service.py` | Wraps C++ interaction functions |
| `recommendation_service.py` | Wraps C++ recommendation functions |
| `dashboard_service.py` | Wraps C++ get_dashboard_stats() |
| `cpp_core.py` | Contract-facing shim and compatibility fallback |
| `src/bindings/biteapple_core.cpp` | Full backup-plan C++ core implementation |

### Routes (Endpoints)

| File | Changes |
|---|---|
| `interactions.py` | Fixed function name from `recent_interactions` to `get_recent_interactions` |

---

## Feature Gaps & Known Limitations

### Missing from Current C++ Implementation

1. **Payment Integration** - Out of scope for C++ core
   - Using Paymob payment gateway
   - Python handles payment flow
   - Not required in C++ contract

2. **Persistent C++ Storage** - The backup C++ core is in-memory
  - Good for the backup plan and local demo
  - Can be replaced later with a database-backed core if needed

3. **Password Hashing Hardening** - The backup core stores credentials in-memory for demo purposes
  - Good enough for the fallback implementation
  - Replace with a stronger auth stack for production use

---

## Setup & Deployment Instructions

### Building the C++ Module

```bash
cd /workspaces/Feed_Sys
mkdir -p build
cd build
cmake ..
make
```

**Output**: `build/biteapple_core.so` (Linux) or `.pyd` (Windows)

### Environment Variable

```bash
export BITEAPPLE_CORE_PATH=/workspaces/Feed_Sys/build
```

Or set in `.env`:
```
BITEAPPLE_CORE_PATH=/workspaces/Feed_Sys/build
```

### Running the Backend

```bash
cd /workspaces/Feed_Sys/backend

# Install dependencies
pip install -r requirements.txt

# Start FastAPI server
uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
```

### Testing
```bash
# Test without C++ module (fallback to SQLAlchemy)
python -m pytest tests/

# Test with C++ module
export BITEAPPLE_CORE_PATH=/workspaces/Feed_Sys/build
python -m pytest tests/
```

---

## Error Handling

### HTTP Status Codes

| Code | Meaning | Example |
|---|---|---|
| 200 | Success | Product retrieved |
| 400 | Bad request | Invalid interaction type |
| 401 | Unauthorized | Wrong password |
| 404 | Not found | User/product doesn't exist |
| 409 | Conflict | Email already registered |
| 500 | Server error | C++ binding not available |

### Response Format

Success:
```json
{
  "id": 1,
  "name": "Product Name",
  ...
}
```

Error:
```json
{
  "detail": "Product not found"
}
```

---

## Interaction Type Mapping

Python → C++ Enum Conversion:

| Python String | C++ Enum | Weight |
|---|---|---|
| "view" | VIEW | 1 |
| "click" | CLICK | 2 |
| "cart" | ADD_TO_CART | 5 |
| "purchase" | PURCHASE | 10 |

---

## Recommendations Algorithm

When C++ `get_recommendations()` is available, it combines:

1. **Item Popularity Score** - Base score from product popularity
2. **Graph Similarity** - Products related to recently interacted items (+10 each)
3. **Category Boost** - User's preferred category (+15 bonus)
4. **Interaction Weight** - Historical interactions weighted by type:
   - View: +1
   - Click: +2
   - Add to cart: +5
   - Purchase: +10

For new users or if C++ unavailable: **Return top products by category**

---

## Graph-Based Similarity

The C++ Graph structure builds relationships between products based on:
- **Co-interactions**: Products viewed/purchased by the same users
- **Category proximity**: Similar categories have higher connection weight
- **Popularity weight**: Popular items influence similarity more

---

## Next Steps for C++ Team

### Phase 1: Core Data Structures
- ✅ Graph class (complete)
- ✅ Heap class (complete)
- ✅ Queue class (complete)
- ✅ HashMap (mention in architecture)

### Phase 2: User Management (Cluster 1)
- [ ] Implement `authenticate_user()` 
  - Password hashing/verification
  - Return AuthResult with UserDTO
- [ ] Implement `create_user()`
  - Email validation
  - Set default scores/level
- [ ] Implement `get_user_by_id()`
  - Return UserDTO with all fields

### Phase 3: Product Management (Cluster 2)
- [ ] Implement `list_products()`
  - Filter by category
  - Return sorted by popularity
- [ ] Implement `get_product_by_id()`
  - Validate ID exists
- [ ] Implement `create_product()`
  - Assign new ID
  - Add to ranking Heap
- [ ] Implement `update_product()`
  - Update fields
  - Rebuild Heap if score changes
- [ ] Implement `delete_product()`
  - Clean up Graph edges
  - Remove from Heap
- [ ] Implement `get_cart()`, `add_to_cart()`, `remove_from_cart()`
  - Cart state management
  - Quantity tracking
- [ ] Implement `checkout()`
  - Convert cart to purchases
  - Record purchase interactions
  - Clear cart
- [ ] Implement `get_dashboard_stats()`
  - Count totals
  - Find most popular category

### Phase 4: Recommendations (Cluster 3)
- [ ] Implement `record_interaction()`
  - Queue interaction
  - Update scores
  - Update Graph weights
- [ ] Implement `get_recent_interactions()`
  - Already have C++ version
  - Verify output format
- [ ] Implement `get_recommendations()`
  - Use improved scoring
  - Graph-based similarity
- [ ] Implement `get_related_products()`
  - Already have C++ version
  - Optimize for performance
- [ ] Implement `get_trending()`
  - Use Heap for efficiency

### Testing Checklist

For each function, provide:
- [ ] Unit tests (C++)
- [ ] Integration tests with pybind11
- [ ] Sample API calls (cURL/Postman)
- [ ] Performance benchmarks
- [ ] Edge case handling

---

## Frontend Compatibility

The frontend expects these `api.js` function names:

```javascript
// These map directly to FastAPI endpoints
login()
signup()
getProducts()
getItem()
getItemsByCategory()
addProduct()
updateProduct()
deleteProduct()
recordInteraction()
getRecentInteractions()
getRecommendations()
getRelatedProducts()
getCart()
addToCart()
removeFromCart()
checkout()
getDashboardStats()
```

All responses use camelCase in the frontend.

---

## Architecture Decisions

### Why Thin Python Wrapper?

1. **Separation of Concerns**
   - C++ handles complex algorithms
   - Python handles HTTP/database layer
   - Easy to test independently

2. **Gradual Migration**
   - Can implement C++ functions incrementally
   - System works with fallback to SQLAlchemy
   - No need to rewrite everything at once

3. **Flexibility**
   - Easy to add new features in either layer
   - Can optimize hot paths in C++
   - Maintains clean API contracts

4. **Team Productivity**
   - Frontend developers: No impact
   - Python team: Minimal changes
   - C++ team: Clear specification of requirements

---

## Troubleshooting

### C++ Module Not Loading

```python
# Check what went wrong:
from app.services import cpp_core
print(cpp_core.load_core())  # Returns None if unavailable

if not cpp_core.is_available():
    print("Using SQLAlchemy fallback")
```

### Function Not Found in C++

If C++ module loads but a specific function is missing:

```python
core = cpp_core.load_core()
if core and hasattr(core, 'function_name'):
    # Function available
    result = core.function_name(...)
else:
    # Use fallback
    result = fallback_implementation(...)
```

### Performance Issues

- Profile C++ functions to find bottlenecks
- Use Heap for ranking (O(n log n) better than sorting O(n^2))
- Use Graph neighbors before falling back to category (O(neighbors) vs O(all products))

---

## References

### Files Modified

- `/workspaces/Feed_Sys/backend/app/services/cpp_core.py`
- `/workspaces/Feed_Sys/backend/app/services/auth_service.py`
- `/workspaces/Feed_Sys/backend/app/services/product_service.py`
- `/workspaces/Feed_Sys/backend/app/services/cart_service.py`
- `/workspaces/Feed_Sys/backend/app/services/interaction_service.py`
- `/workspaces/Feed_Sys/backend/app/services/recommendation_service.py`
- `/workspaces/Feed_Sys/backend/app/services/dashboard_service.py`
- `/workspaces/Feed_Sys/backend/app/routes/interactions.py`

### Contract Document

Reference: `docs/backend-api-cpp-contract.md` (provided as attachment)

### C++ Source Files

- `/workspaces/Feed_Sys/src/bindings/biteapple_core.cpp` (current binding)
- `/workspaces/Feed_Sys/src/models/` (User, Item, Interaction classes)
- `/workspaces/Feed_Sys/src/data_structures/` (Graph, Heap, Queue, HashMap)

---

## Version History

| Version | Date | Changes |
|---|---|---|
| 1.0 | 2026-05-09 | Initial refactoring to match C++ contract |

---

**Next Review Date**: After C++ team completes Cluster 1 functions  
**Maintainer**: Backend Team  
**Last Updated**: 2026-05-09
