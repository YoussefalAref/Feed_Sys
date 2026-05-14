#include "../Vector.h"
#include "../HashMap.h"
#include "../STLHelper.h"
#include "../Graph.h"
#include "../Heap.h"
#include "../Queue.h"
#include "../Interaction.h"
#include "../DTOs.h"
#include "../StateStructs.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

// Type caster: Vector<T> <-> Python list (must come before any function definitions)
namespace pybind11 { namespace detail {
    template <typename T>
    struct type_caster<Vector<T>> {
        PYBIND11_TYPE_CASTER(Vector<T>, _("List"));

        bool load(handle src, bool convert) {
            if (!isinstance<sequence>(src) || isinstance<str>(src) || isinstance<bytes>(src))
                return false;
            value.clear();
            for (const auto& item : reinterpret_borrow<sequence>(src)) {
                value.push_back(item.template cast<T>());
            }
            return true;
        }

        static handle cast(const Vector<T>& src, return_value_policy /*policy*/, handle /*parent*/) {
            list lst;
            for (int i = 0; i < src.getSize(); i++)
                lst.append(src[i]);
            return lst.release();
        }
    };
}} // namespace pybind11::detail

namespace {

// Global state variables
HashMap<int, UserState> g_users;
HashMap<int, ProductState> g_products;
HashMap<std::string, CartEntry> g_cart;
Vector<InteractionState> g_interactions;
int g_nextUserId = 1;
int g_nextProductId = 101;
int g_nextInteractionId = 1;
bool g_seeded = false;
Queue g_interactionQueue;

std::string nowIso8601() {
    std::time_t now = std::time(nullptr);
    std::tm utcTime{};
#if defined(_WIN32)
    gmtime_s(&utcTime, &now);
#else
    gmtime_r(&now, &utcTime);
#endif
    char buffer[32] = {0};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utcTime);
    return buffer;
}

std::string interactionTypeCanonical(const std::string& type) {
    std::string normalized = stringToLower(type);
    if (normalized == "view") return "view";
    if (normalized == "click") return "click";
    if (normalized == "cart" || normalized == "add_to_cart") return "cart";
    if (normalized == "purchase") return "purchase";
    return "";
}

int interactionWeight(const std::string& type) {
    std::string normalized = interactionTypeCanonical(type);
    if (normalized == "view") return 1;
    if (normalized == "click") return 2;
    if (normalized == "cart") return 5;
    if (normalized == "purchase") return 10;
    return 0;
}

std::string userLevelFromScore(int activityScore) {
    if (activityScore > 1000) return "VIP";
    if (activityScore > 500) return "Active";
    return "Normal";
}

std::string cartKey(int userId, int itemId) {
    return intToString(userId) + ":" + intToString(itemId);
}

py::dict productToDict(const ProductState& product) {
    py::dict result;
    result["id"] = product.id;
    result["name"] = product.name;
    result["price"] = product.price;
    result["category"] = product.category;
    result["popularity_score"] = product.popularity_score;
    result["stock"] = product.stock;
    result["image"] = product.image;
    result["description"] = product.description;
    return result;
}

py::dict userToDict(const UserState& user) {
    py::dict result;
    result["id"] = user.id;
    result["name"] = user.name;
    result["email"] = user.email;
    result["category"] = user.category;
    result["score"] = user.score;
    result["region"] = user.region;
    result["activity_score"] = user.activity_score;
    result["level"] = user.level;
    return result;
}

py::dict interactionToDict(const InteractionState& interaction) {
    py::dict result;
    result["id"] = interaction.id;
    result["user_id"] = interaction.user_id;
    result["item_id"] = interaction.item_id;
    result["type"] = interaction.type;
    result["timestamp"] = interaction.timestamp;
    return result;
}

py::dict cartItemToDict(const CartEntry& entry) {
    py::dict result;
    result["user_id"] = entry.user_id;
    result["item_id"] = entry.item_id;
    result["quantity"] = entry.quantity;

    ProductState* productPtr = g_products.searchPointer(entry.item_id);
    if (productPtr != nullptr) {
        result["product"] = productToDict(*productPtr);
    } else {
        result["product"] = py::dict();
    }

    return result;
}

UserDTO userStateToDTO(const UserState& s) {
    UserDTO d;
    d.id             = s.id;
    d.name           = s.name;
    d.email          = s.email;
    d.category       = s.category;
    d.score          = s.score;
    d.region         = s.region;
    d.activity_score = s.activity_score;
    d.level          = s.level;
    return d;
}

py::dict userDTOToDict(const UserDTO& u) {
    py::dict result;
    result["id"]             = u.id;
    result["name"]           = u.name;
    result["email"]          = u.email;
    result["category"]       = u.category;
    result["score"]          = u.score;
    result["region"]         = u.region;
    result["activity_score"] = u.activity_score;
    result["level"]          = u.level;
    return result;
}

py::dict authResultToDict(const AuthResult& authResult) {
    py::dict result;
    result["success"] = authResult.success;
    result["error"]   = authResult.error;
    result["user"]    = userDTOToDict(authResult.user);
    result["token"]   = authResult.token;
    return result;
}

py::dict checkoutResultToDict(const CheckoutResult& checkoutResult) {
    py::dict result;
    result["success"] = checkoutResult.success;
    result["purchased"] = checkoutResult.purchased;
    return result;
}

py::dict dashboardStatsToDict(const DashboardStats& stats) {
    py::dict result;
    result["totalProducts"] = stats.totalProducts;
    result["totalUsers"] = stats.totalUsers;
    result["totalInteractions"] = stats.totalInteractions;
    result["mostPopularCategory"] = stats.mostPopularCategory;
    return result;
}

ProductState* findProduct(int id) {
    return g_products.searchPointer(id);
}

UserState* findUserById(int id) {
    return g_users.searchPointer(id);
}

UserState* findUserByEmail(const std::string& email) {
    std::string normalizedEmail = stringToLower(email);
    Vector<int> allIds = g_users.getAllKeys();
    for (int i = 0; i < allIds.getSize(); ++i) {
        UserState* user = g_users.searchPointer(allIds[i]);
        if (user && stringToLower(user->email) == normalizedEmail) {
            return user;
        }
    }
    return nullptr;
}

void seedDefaults() {
    if (g_seeded) return;

    g_users.clear();
    g_products.clear();
    g_cart.clear();
    g_interactions.clear();

    g_users.insert(1, UserState{1, "Mariam Saleh", "mariam@biteapple.test", "password123", "Electronics", "Cairo", 92.0, 8, "Normal"});
    g_users.insert(2, UserState{2, "Omar Hassan", "omar@biteapple.test", "password123", "Fitness", "Alexandria", 76.0, 12, "Normal"});
    g_users.insert(3, UserState{3, "Manager One", "manager@biteapple.test", "admin123", "Business", "Giza", 98.0, 20, "Normal"});

    g_products.insert(101, ProductState{101, "AirPods Pro MaxCase", 2499.0, "Electronics", 94.0, 18, "", "Protective case for premium earbuds."});
    g_products.insert(102, ProductState{102, "Smart Fitness Band", 1299.0, "Fitness", 87.0, 31, "", "Activity tracker with heart-rate monitoring."});
    g_products.insert(103, ProductState{103, "Minimal Desk Lamp", 850.0, "Home", 78.0, 24, "", "LED desk lamp with warm light."});
    g_products.insert(104, ProductState{104, "Campus Backpack", 1100.0, "Fashion", 83.0, 15, "", "Durable backpack for daily use."});
    g_products.insert(105, ProductState{105, "Cold Brew Kit", 690.0, "Kitchen", 72.0, 22, "", "Simple glass cold brew maker."});
    g_products.insert(106, ProductState{106, "Wireless Keyboard", 1650.0, "Electronics", 91.0, 12, "", "Low-profile wireless keyboard."});
    g_products.insert(107, ProductState{107, "Yoga Recovery Mat", 780.0, "Fitness", 69.0, 27, "", "Supportive mat for recovery sessions."});
    g_products.insert(108, ProductState{108, "Ceramic Dinner Set", 1850.0, "Home", 81.0, 9, "", "Six-piece dinner set."});
    g_products.insert(109, ProductState{109, "Running Shoes", 1450.0, "Fitness", 74.0, 16, "", "Lightweight running shoes."});
    g_products.insert(110, ProductState{110, "Espresso Grinder", 2100.0, "Kitchen", 79.0, 8, "", "Compact burr grinder."});

    g_nextUserId = 4;
    g_nextProductId = 111;
    g_nextInteractionId = 1;
    g_seeded = true;
}

Vector<ProductState> allProductsSorted() {
    Vector<ProductState> result;
    result.reserve(g_products.getSize());

    Vector<int> allIds = g_products.getAllKeys();
    for (int i = 0; i < allIds.getSize(); ++i) {
        ProductState* product = g_products.searchPointer(allIds[i]);
        if (product) result.push_back(*product);
    }

    // Bubble sort by popularity score descending, then by id ascending
    int n = result.getSize();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            bool shouldSwap = (result[j].popularity_score == result[j+1].popularity_score)
                ? (result[j].id > result[j+1].id)
                : (result[j].popularity_score < result[j+1].popularity_score);
            if (shouldSwap) {
                ProductState temp = result[j];
                result[j] = result[j+1];
                result[j+1] = temp;
            }
        }
    }
    return result;
}

Vector<ProductState> filterProductsByCategory(const std::string& category) {
    Vector<ProductState> result;
    std::string normalized = stringToLower(category);

    Vector<int> allIds = g_products.getAllKeys();
    for (int i = 0; i < allIds.getSize(); ++i) {
        ProductState* product = g_products.searchPointer(allIds[i]);
        if (product && (normalized.empty() || stringToLower(product->category) == normalized))
            result.push_back(*product);
    }

    int n = result.getSize();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            bool shouldSwap = (result[j].popularity_score == result[j+1].popularity_score)
                ? (result[j].id > result[j+1].id)
                : (result[j].popularity_score < result[j+1].popularity_score);
            if (shouldSwap) {
                ProductState temp = result[j];
                result[j] = result[j+1];
                result[j+1] = temp;
            }
        }
    }
    return result;
}

void updateUserActivity(UserState& user, const std::string& type) {
    user.activity_score += interactionWeight(type);
    user.level = userLevelFromScore(user.activity_score);
}

void updateProductPopularity(ProductState& product, const std::string& type) {
    product.popularity_score += interactionWeight(type);
}

InteractionType toInteractionType(const std::string& canonical) {
    if (canonical == "click")    return CLICK;
    if (canonical == "cart")     return ADD_TO_CART;
    if (canonical == "purchase") return PURCHASE;
    return VIEW;
}

// Drain g_interactionQueue and apply score effects for each event.
void processInteractionQueue() {
    while (!g_interactionQueue.isEmpty()) {
        Interaction event = g_interactionQueue.dequeue();

        std::string typeStr;
        switch (event.getType()) {
            case VIEW:        typeStr = "view";     break;
            case CLICK:       typeStr = "click";    break;
            case ADD_TO_CART: typeStr = "cart";     break;
            case PURCHASE:    typeStr = "purchase"; break;
        }

        UserState*    user    = findUserById(event.getUserID());
        ProductState* product = findProduct(event.getItemID());
        if (user)    updateUserActivity(*user, typeStr);
        if (product) updateProductPopularity(*product, typeStr);
    }
}

InteractionState appendInteraction(int userId, int itemId, const std::string& type) {
    InteractionState interaction;
    interaction.id = g_nextInteractionId++;
    interaction.user_id = userId;
    interaction.item_id = itemId;
    interaction.type = interactionTypeCanonical(type);
    interaction.timestamp = nowIso8601();
    g_interactions.push_back(interaction);
    return interaction;
}

InteractionState recordInteractionInternal(int userId, int itemId, const std::string& type) {
    seedDefaults();

    UserState* user = findUserById(userId);
    ProductState* product = findProduct(itemId);
    std::string canonicalType = interactionTypeCanonical(type);

    if (!user)    throw std::runtime_error("User not found");
    if (!product) throw std::runtime_error("Product not found");
    if (canonicalType.empty()) throw std::runtime_error("Unsupported interaction type");

    // Stage the event in the queue, then flush — score updates happen inside processInteractionQueue.
    g_interactionQueue.enqueue(Interaction(userId, itemId, toInteractionType(canonicalType)));
    InteractionState interaction = appendInteraction(userId, itemId, canonicalType);
    processInteractionQueue();
    return interaction;
}

// Extract top `limit` products by popularity score using a max-heap.
// O(n + k log n) — faster than sorting everything when k << n.
Vector<py::dict> trendingFromProducts(const Vector<ProductState>& products, int limit) {
    if (products.empty() || limit <= 0) return {};

    // Build Item array and insert into max-heap
    Heap heap(products.getSize());
    for (int i = 0; i < products.getSize(); ++i) {
        const ProductState& p = products[i];
        heap.insert(Item(intToString(p.id), p.name, p.price, p.category,
                         static_cast<int>(p.popularity_score), p.stock));
    }

    // Extract top-k items; look up the full ProductState for each
    Vector<py::dict> result;
    int safeLimit = maxVal(0, limit);
    while (!heap.isEmpty() && static_cast<int>(result.getSize()) < safeLimit) {
        Item top = heap.extractMax();
        int productId = stringToInt(top.getID());
        ProductState* ptr = g_products.searchPointer(productId);
        if (ptr != nullptr) {
            result.push_back(productToDict(*ptr));
        } else {
            // Fallback: build dict directly from the Item (used by legacy overload)
            ProductState ps;
            ps.id               = productId;
            ps.name             = top.getName();
            ps.price            = top.getPrice();
            ps.category         = top.getCategory();
            ps.popularity_score = static_cast<double>(top.getPopularityScore());
            ps.stock            = top.getStock();
            result.push_back(productToDict(ps));
        }
    }
    return result;
}

Vector<py::dict> relatedProductsFromProducts(const Vector<ProductState>& products, int itemId, int limit) {
    Vector<Item> items;
    items.reserve(products.getSize());
    for (int i = 0; i < products.getSize(); ++i) {
        const ProductState& p = products[i];
        items.push_back(Item(intToString(p.id), p.name, p.price, p.category,
                             static_cast<int>(p.popularity_score), p.stock));
    }

    if (items.empty()) return {};

    Graph graph;
    graph.build(items.data(), static_cast<int>(items.getSize()));

    int count = 0;
    std::string* neighbors = graph.getNeighbors(intToString(itemId), count);
    if (neighbors == nullptr) return {};

    Vector<py::dict> result;
    int safeLimit = maxVal(0, limit);
    for (int i = 0; i < count && static_cast<int>(result.getSize()) < safeLimit; ++i) {
        int neighborId = 0;
        try { neighborId = stringToInt(neighbors[i]); } catch (...) { continue; }

        ProductState* productPtr = g_products.searchPointer(neighborId);
        if (productPtr == nullptr) continue;

        py::dict payload = productToDict(*productPtr);
        payload["similarity_score"] = 0.87;
        result.push_back(payload);
    }
    return result;
}

Vector<py::dict> recommendationsFromProducts(
    const Vector<ProductState>& products,
    const Vector<InteractionState>& interactions,
    const UserState& user,
    int limit
) {
    if (products.empty()) return {};

    Vector<Item> items;
    items.reserve(products.getSize());
    for (int i = 0; i < products.getSize(); ++i) {
        const ProductState& p = products[i];
        items.push_back(Item(intToString(p.id), p.name, p.price, p.category,
                             static_cast<int>(p.popularity_score), p.stock));
    }

    Graph graph;
    graph.build(items.data(), static_cast<int>(items.getSize()));

    Vector<std::string> interactedIds;
    for (int i = 0; i < interactions.getSize(); ++i) {
        const InteractionState& interaction = interactions[i];
        if (interaction.user_id == user.id) {
            std::string itemId = intToString(interaction.item_id);
            if (vectorFind(interactedIds, itemId) == -1) {
                interactedIds.push_back(itemId);
            }
        }
    }

    Vector<Pair<py::dict>> scored;
    for (int pi = 0; pi < products.getSize(); ++pi) {
        const ProductState& product = products[pi];
        if (product.stock <= 0) continue;

        double score = product.popularity_score;
        if (stringToLower(product.category) == stringToLower(user.category)) score += 15.0;

        for (int ii = 0; ii < interactedIds.getSize(); ++ii) {
            int count = 0;
            std::string* neighbors = graph.getNeighbors(interactedIds[ii], count);
            if (neighbors == nullptr) continue;
            for (int n = 0; n < count; ++n) {
                if (neighbors[n] == intToString(product.id)) { score += 10.0; break; }
            }
        }

        for (int ii = 0; ii < interactions.getSize(); ++ii) {
            const InteractionState& interaction = interactions[ii];
            if (interaction.user_id == user.id && interaction.item_id == product.id)
                score += static_cast<double>(interactionWeight(interaction.type));
        }

        py::dict payload = productToDict(product);
        payload["recommendation_score"] = score;

        std::string reason = "Because it matches your interests";
        if (stringToLower(product.category) == stringToLower(user.category))
            reason = "Because you interacted with " + product.category + " products";
        payload["reason"] = reason;

        scored.push_back(Pair<py::dict>(payload, score));
    }

    vectorSort(scored, [](const Pair<py::dict>& left, const Pair<py::dict>& right) {
        if (left.second == right.second) {
            return py::cast<int>(left.first["id"]) < py::cast<int>(right.first["id"]);
        }
        return left.second > right.second;
    });

    Vector<py::dict> result;
    int safeLimit = maxVal(0, limit);
    for (int i = 0; i < safeLimit && i < static_cast<int>(scored.getSize()); ++i) {
        result.push_back(scored[i].first);
    }
    return result;
}

Vector<py::dict> recentInteractionsFromList(const Vector<InteractionState>& interactions, int limit) {
    Vector<py::dict> result;
    int safeLimit = maxVal(0, limit);
    for (auto it = interactions.rbegin();
         it != interactions.rend() && static_cast<int>(result.getSize()) < safeLimit;
         ++it) {
        result.push_back(interactionToDict(*it));
    }
    return result;
}

}  // namespace

// ---------------------------------------------------------------------------
// Contract functions used by FastAPI / pybind
// ---------------------------------------------------------------------------

py::dict authenticate_user(const std::string& email, const std::string& password) {
    seedDefaults();

    AuthResult result;
    UserState* user = findUserByEmail(email);
    if (!user) {
        result.success = false;
        result.error = "Invalid email or password";
        return authResultToDict(result);
    }

    if (user->password != password) {
        result.success = false;
        result.error = "Invalid email or password";
        return authResultToDict(result);
    }

    result.success = true;
    result.user    = userStateToDTO(*user);
    result.token   = "dev-token-" + intToString(user->id);
    return authResultToDict(result);
}

py::dict create_user(const py::dict& input) {
    seedDefaults();

    std::string name     = py::cast<std::string>(input["name"]);
    std::string email    = py::cast<std::string>(input["email"]);
    std::string password = py::cast<std::string>(input["password"]);
    std::string category = input.contains("category") ? py::cast<std::string>(input["category"]) : "Electronics";
    std::string region   = input.contains("region")   ? py::cast<std::string>(input["region"])   : "Cairo";

    if (findUserByEmail(email)) throw std::runtime_error("Email is already registered");

    UserState user;
    user.id             = g_nextUserId++;
    user.name           = name;
    user.email          = email;
    user.password       = password;
    user.category       = category;
    user.region         = region;
    user.score          = 50.0;
    user.activity_score = 0;
    user.level          = "Normal";
    g_users.insert(user.id, user);

    AuthResult result;
    result.success = true;
    result.user    = userStateToDTO(user);
    result.token   = "dev-token-" + intToString(user.id);
    return authResultToDict(result);
}

py::dict get_user_by_id(int user_id) {
    seedDefaults();
    UserState* user = findUserById(user_id);
    if (!user) throw std::runtime_error("User not found");
    return userToDict(*user);
}

Vector<py::dict> list_products(const std::string& category) {
    seedDefaults();
    Vector<py::dict> result;
    for (const auto& product : filterProductsByCategory(category))
        result.push_back(productToDict(product));
    return result;
}

py::dict get_product_by_id(int item_id) {
    seedDefaults();
    ProductState* product = findProduct(item_id);
    if (!product) throw std::runtime_error("Product not found");
    return productToDict(*product);
}

py::dict create_product(const py::dict& input) {
    seedDefaults();

    ProductState product;
    product.id               = g_nextProductId++;
    product.name             = py::cast<std::string>(input["name"]);
    product.price            = py::cast<double>(input["price"]);
    product.category         = py::cast<std::string>(input["category"]);
    product.stock            = input.contains("stock")            ? py::cast<int>(input["stock"])            : 0;
    product.image            = input.contains("image")            ? py::cast<std::string>(input["image"])    : "";
    product.description      = input.contains("description")      ? py::cast<std::string>(input["description"]) : "";
    product.popularity_score = input.contains("popularity_score") ? py::cast<double>(input["popularity_score"]) : 40.0;
    g_products.insert(product.id, product);

    return productToDict(product);
}

py::dict update_product(int item_id, const py::dict& input) {
    seedDefaults();
    ProductState* product = findProduct(item_id);
    if (!product) throw std::runtime_error("Product not found");

    product->name     = py::cast<std::string>(input["name"]);
    product->price    = py::cast<double>(input["price"]);
    product->category = py::cast<std::string>(input["category"]);
    if (input.contains("stock"))            product->stock            = py::cast<int>(input["stock"]);
    if (input.contains("image"))            product->image            = py::cast<std::string>(input["image"]);
    if (input.contains("description"))      product->description      = py::cast<std::string>(input["description"]);
    if (input.contains("popularity_score")) product->popularity_score = py::cast<double>(input["popularity_score"]);

    return productToDict(*product);
}

bool delete_product(int item_id) {
    seedDefaults();
    if (!g_products.contains(item_id)) return false;
    g_products.remove(item_id);

    Vector<std::string> keysToRemove;
    Vector<std::string> allCartKeys = g_cart.getAllKeys();
    for (int i = 0; i < allCartKeys.getSize(); ++i) {
        CartEntry entry = g_cart.search(allCartKeys[i]);
        if (entry.item_id == item_id) keysToRemove.push_back(allCartKeys[i]);
    }
    for (int j = 0; j < keysToRemove.getSize(); ++j)
        g_cart.remove(keysToRemove[j]);

    return true;
}

Vector<py::dict> get_cart(int user_id) {
    seedDefaults();
    Vector<py::dict> result;
    Vector<std::string> allCartKeys = g_cart.getAllKeys();
    for (int i = 0; i < allCartKeys.getSize(); ++i) {
        CartEntry entry = g_cart.search(allCartKeys[i]);
        if (entry.user_id == user_id) result.push_back(cartItemToDict(entry));
    }
    vectorSort(result, [](const py::dict& left, const py::dict& right) {
        return py::cast<int>(left["item_id"]) < py::cast<int>(right["item_id"]);
    });
    return result;
}

bool add_to_cart(int user_id, int item_id, int quantity) {
    seedDefaults();
    if (quantity < 1) throw std::runtime_error("Quantity must be at least 1");
    if (!findUserById(user_id))  throw std::runtime_error("User not found");
    if (!findProduct(item_id))   throw std::runtime_error("Product not found");

    std::string key = cartKey(user_id, item_id);
    CartEntry* existing = g_cart.searchPointer(key);
    if (existing == nullptr) {
        g_cart.insert(key, CartEntry{user_id, item_id, quantity});
    } else {
        existing->quantity += quantity;
    }

    recordInteractionInternal(user_id, item_id, "cart");
    return true;
}

bool remove_from_cart(int user_id, int item_id) {
    seedDefaults();
    std::string key = cartKey(user_id, item_id);
    if (!g_cart.contains(key)) return false;
    g_cart.remove(key);
    return true;
}

py::dict checkout(int user_id) {
    seedDefaults();
    if (!findUserById(user_id)) throw std::runtime_error("User not found");

    Vector<CartEntry> entries;
    Vector<std::string> allCartKeys = g_cart.getAllKeys();
    for (int i = 0; i < allCartKeys.getSize(); ++i) {
        CartEntry entry = g_cart.search(allCartKeys[i]);
        if (entry.user_id == user_id) entries.push_back(entry);
    }

    if (entries.empty()) throw std::runtime_error("Cart is empty");

    int purchased = 0;
    for (int i = 0; i < entries.getSize(); ++i) {
        const CartEntry& entry = entries[i];
        ProductState* product = findProduct(entry.item_id);
        if (!product) continue;

        int quantity = maxVal(1, entry.quantity);
        if (product->stock >= quantity) product->stock -= quantity;
        else product->stock = 0;

        for (int q = 0; q < quantity; ++q)
            recordInteractionInternal(user_id, entry.item_id, "purchase");

        g_cart.remove(cartKey(entry.user_id, entry.item_id));
        purchased += quantity;
    }

    CheckoutResult result;
    result.success   = true;
    result.purchased = purchased;
    return checkoutResultToDict(result);
}

py::dict get_dashboard_stats() {
    seedDefaults();
    DashboardStats stats;
    stats.totalProducts     = static_cast<int>(g_products.getSize());
    stats.totalUsers        = static_cast<int>(g_users.getSize());
    stats.totalInteractions = static_cast<int>(g_interactions.getSize());

    HashMap<std::string, double> categoryTotals;
    Vector<int> productIds = g_products.getAllKeys();
    for (int i = 0; i < productIds.getSize(); ++i) {
        ProductState* product = g_products.searchPointer(productIds[i]);
        if (product) {
            double current = categoryTotals.search(product->category);
            categoryTotals.insert(product->category, current + product->popularity_score);
        }
    }

    double bestScore = -1.0;
    Vector<std::string> categories = categoryTotals.getAllKeys();
    for (int i = 0; i < categories.getSize(); ++i) {
        double total = categoryTotals.search(categories[i]);
        if (total > bestScore) {
            bestScore = total;
            stats.mostPopularCategory = categories[i];
        }
    }

    return dashboardStatsToDict(stats);
}

py::dict record_interaction(int user_id, int item_id, const std::string& interaction_type) {
    InteractionState interaction = recordInteractionInternal(user_id, item_id, interaction_type);
    return interactionToDict(interaction);
}

Vector<py::dict> get_recent_interactions(int limit) {
    seedDefaults();
    Vector<InteractionState> ordered = g_interactions;
    std::reverse(ordered.begin(), ordered.end());
    return recentInteractionsFromList(ordered, limit);
}

Vector<py::dict> get_recommendations(int user_id, int limit) {
    seedDefaults();
    UserState* user = findUserById(user_id);
    if (!user) return trendingFromProducts(allProductsSorted(), limit);

    Vector<InteractionState> userInteractions;
    for (int i = 0; i < g_interactions.getSize(); ++i) {
        if (g_interactions[i].user_id == user_id)
            userInteractions.push_back(g_interactions[i]);
    }

    return recommendationsFromProducts(allProductsSorted(), userInteractions, *user, limit);
}

Vector<py::dict> get_related_products(int item_id, int limit) {
    seedDefaults();
    ProductState* product = findProduct(item_id);
    if (!product) return {};

    Vector<ProductState> products = allProductsSorted();
    auto related = relatedProductsFromProducts(products, item_id, limit);
    if (!related.empty()) return related;

    Vector<py::dict> fallback;
    for (int i = 0; i < products.getSize(); ++i) {
        const ProductState& candidate = products[i];
        if (candidate.id == item_id) continue;
        if (stringToLower(candidate.category) == stringToLower(product->category)) {
            py::dict payload = productToDict(candidate);
            payload["similarity_score"] = 0.75;
            fallback.push_back(payload);
        }
    }

    vectorSort(fallback, [](const py::dict& left, const py::dict& right) {
        double ls = py::cast<double>(left["similarity_score"]);
        double rs = py::cast<double>(right["similarity_score"]);
        if (ls == rs) return py::cast<int>(left["id"]) < py::cast<int>(right["id"]);
        return ls > rs;
    });

    Vector<py::dict> result;
    for (int i = 0; i < fallback.getSize() && i < limit; ++i)
        result.push_back(fallback[i]);
    return result;
}

Vector<py::dict> get_trending(int limit) {
    seedDefaults();
    return trendingFromProducts(allProductsSorted(), limit);
}

// ---------------------------------------------------------------------------
// Legacy compatibility overloads for current Python wrappers
// ---------------------------------------------------------------------------

Vector<py::dict> rank_top_products(const Vector<py::dict>& products, int limit) {
    Vector<ProductState> items;
    items.reserve(products.getSize());
    for (int idx = 0; idx < products.getSize(); ++idx) {
        const py::dict& p = products[idx];
        ProductState item;
        item.id               = py::cast<int>(p["id"]);
        item.name             = py::cast<std::string>(p["name"]);
        item.price            = py::cast<double>(p["price"]);
        item.category         = py::cast<std::string>(p["category"]);
        item.popularity_score = py::cast<double>(p["popularity_score"]);
        item.stock            = py::cast<int>(p["stock"]);
        if (p.contains("image"))       item.image       = py::cast<std::string>(p["image"]);
        if (p.contains("description")) item.description = py::cast<std::string>(p["description"]);
        items.push_back(item);
    }
    return trendingFromProducts(items, limit);
}

Vector<py::dict> get_related_products(const Vector<py::dict>& products, int item_id, int limit) {
    Vector<ProductState> items;
    items.reserve(products.getSize());
    for (int idx = 0; idx < products.getSize(); ++idx) {
        const py::dict& p = products[idx];
        ProductState item;
        item.id               = py::cast<int>(p["id"]);
        item.name             = py::cast<std::string>(p["name"]);
        item.price            = py::cast<double>(p["price"]);
        item.category         = py::cast<std::string>(p["category"]);
        item.popularity_score = py::cast<double>(p["popularity_score"]);
        item.stock            = py::cast<int>(p["stock"]);
        if (p.contains("image"))       item.image       = py::cast<std::string>(p["image"]);
        if (p.contains("description")) item.description = py::cast<std::string>(p["description"]);
        items.push_back(item);
    }
    return relatedProductsFromProducts(items, item_id, limit);
}

Vector<py::dict> get_recent_interactions(const Vector<py::dict>& interactions, int limit) {
    Vector<InteractionState> parsed;
    parsed.reserve(interactions.getSize());
    for (int idx = 0; idx < interactions.getSize(); ++idx) {
        const py::dict& interaction = interactions[idx];
        InteractionState item;
        item.id        = py::cast<int>(interaction["id"]);
        item.user_id   = py::cast<int>(interaction["user_id"]);
        item.item_id   = py::cast<int>(interaction["item_id"]);
        item.type      = interaction.contains("type")      ? py::cast<std::string>(interaction["type"])      : "view";
        item.timestamp = interaction.contains("timestamp") ? py::cast<std::string>(interaction["timestamp"]) : nowIso8601();
        parsed.push_back(item);
    }
    Vector<InteractionState> reversed;
    for (int i = parsed.getSize() - 1; i >= 0; --i)
        reversed.push_back(parsed[i]);
    return recentInteractionsFromList(reversed, limit);
}

Vector<py::dict> get_recommendations(const Vector<py::dict>& products,
                                     const Vector<py::dict>& interactions,
                                     const py::dict& user,
                                     int limit) {
    Vector<ProductState> parsedProducts;
    parsedProducts.reserve(products.getSize());
    for (int idx = 0; idx < products.getSize(); ++idx) {
        const py::dict& p = products[idx];
        ProductState item;
        item.id               = py::cast<int>(p["id"]);
        item.name             = py::cast<std::string>(p["name"]);
        item.price            = py::cast<double>(p["price"]);
        item.category         = py::cast<std::string>(p["category"]);
        item.popularity_score = py::cast<double>(p["popularity_score"]);
        item.stock            = py::cast<int>(p["stock"]);
        if (p.contains("image"))       item.image       = py::cast<std::string>(p["image"]);
        if (p.contains("description")) item.description = py::cast<std::string>(p["description"]);
        parsedProducts.push_back(item);
    }

    Vector<InteractionState> parsedInteractions;
    parsedInteractions.reserve(interactions.getSize());
    for (int idx = 0; idx < interactions.getSize(); ++idx) {
        const py::dict& interaction = interactions[idx];
        InteractionState item;
        item.id        = py::cast<int>(interaction["id"]);
        item.user_id   = py::cast<int>(interaction["user_id"]);
        item.item_id   = py::cast<int>(interaction["item_id"]);
        item.type      = interaction.contains("type")      ? py::cast<std::string>(interaction["type"])      : "view";
        item.timestamp = interaction.contains("timestamp") ? py::cast<std::string>(interaction["timestamp"]) : nowIso8601();
        parsedInteractions.push_back(item);
    }

    UserState parsedUser;
    parsedUser.id       = py::cast<int>(user["id"]);
    parsedUser.name     = user.contains("name")     ? py::cast<std::string>(user["name"])     : "";
    parsedUser.email    = user.contains("email")    ? py::cast<std::string>(user["email"])    : "";
    parsedUser.category = user.contains("category") ? py::cast<std::string>(user["category"]) : "";

    return recommendationsFromProducts(parsedProducts, parsedInteractions, parsedUser, limit);
}

Vector<py::dict> score_recommendations(const Vector<py::dict>& products,
                                       const Vector<py::dict>& interactions,
                                       const py::dict& user,
                                       int limit) {
    return get_recommendations(products, interactions, user, limit);
}

// ---------------------------------------------------------------------------
// Pybind module
// ---------------------------------------------------------------------------

PYBIND11_MODULE(biteapple_core, m) {
    m.doc() = "BiteApple C++ backup-plan core bindings";

    m.def("authenticate_user", &authenticate_user, py::arg("email"), py::arg("password"));
    m.def("create_user",       &create_user,       py::arg("input"));
    m.def("get_user_by_id",    &get_user_by_id,    py::arg("user_id"));

    m.def("list_products",     &list_products,     py::arg("category") = "");
    m.def("get_product_by_id", &get_product_by_id, py::arg("item_id"));
    m.def("create_product",    &create_product,    py::arg("input"));
    m.def("update_product",    &update_product,    py::arg("item_id"), py::arg("input"));
    m.def("delete_product",    &delete_product,    py::arg("item_id"));
    m.def("get_cart",          &get_cart,          py::arg("user_id"));
    m.def("add_to_cart",       &add_to_cart,       py::arg("user_id"), py::arg("item_id"), py::arg("quantity") = 1);
    m.def("remove_from_cart",  &remove_from_cart,  py::arg("user_id"), py::arg("item_id"));
    m.def("checkout",          &checkout,          py::arg("user_id"));
    m.def("get_dashboard_stats", &get_dashboard_stats);

    m.def("record_interaction",     &record_interaction,     py::arg("user_id"), py::arg("item_id"), py::arg("interaction_type"));
    m.def("get_recent_interactions", py::overload_cast<int>(&get_recent_interactions),                                  py::arg("limit") = 8);
    m.def("get_recent_interactions", py::overload_cast<const Vector<py::dict>&, int>(&get_recent_interactions),         py::arg("interactions"), py::arg("limit") = 8);

    m.def("get_recommendations",  py::overload_cast<int, int>(&get_recommendations),                                                                     py::arg("user_id"),    py::arg("limit") = 4);
    m.def("get_recommendations",  py::overload_cast<const Vector<py::dict>&, const Vector<py::dict>&, const py::dict&, int>(&get_recommendations),        py::arg("products"),   py::arg("interactions"), py::arg("user"), py::arg("limit") = 4);

    m.def("get_related_products", py::overload_cast<int, int>(&get_related_products),                                   py::arg("item_id"),    py::arg("limit") = 3);
    m.def("get_related_products", py::overload_cast<const Vector<py::dict>&, int, int>(&get_related_products),          py::arg("products"),   py::arg("item_id"), py::arg("limit") = 3);

    m.def("get_trending",         py::overload_cast<int>(&get_trending),                                                py::arg("limit") = 8);
    m.def("get_trending",         py::overload_cast<const Vector<py::dict>&, int>(&rank_top_products),                  py::arg("products"),   py::arg("limit") = 8);

    m.def("rank_top_products",    &rank_top_products,    py::arg("products"), py::arg("limit") = 5);
    m.def("score_recommendations",&score_recommendations, py::arg("products"), py::arg("interactions"), py::arg("user"), py::arg("limit") = 4);
}
