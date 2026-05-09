#include "../data_structures/Graph.h"
#include "../data_structures/Heap.h"
#include "../data_structures/Queue.h"
#include "../models/Interaction.h"
#include "../models/Item.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <functional>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace {
struct UserState {
    int id = -1;
    std::string name;
    std::string email;
    std::string password;
    std::string category;
    std::string region;
    double score = 50.0;
    int activity_score = 0;
    std::string level = "Normal";
};

struct ProductState {
    int id = -1;
    std::string name;
    double price = 0.0;
    std::string category;
    double popularity_score = 0.0;
    int stock = 0;
    std::string image;
    std::string description;
};

struct CartEntry {
    int user_id = -1;
    int item_id = -1;
    int quantity = 0;
};

struct InteractionState {
    int id = -1;
    int user_id = -1;
    int item_id = -1;
    std::string type;
    std::string timestamp;
};

struct AuthResult {
    bool success = false;
    std::string error;
    UserState user;
    std::string token;
};

struct CheckoutResult {
    bool success = false;
    int purchased = 0;
};

struct DashboardStats {
    int totalProducts = 0;
    int totalUsers = 0;
    int totalInteractions = 0;
    std::string mostPopularCategory = "N/A";
};

std::unordered_map<int, UserState> g_users;
std::unordered_map<int, ProductState> g_products;
std::unordered_map<std::string, CartEntry> g_cart;
std::vector<InteractionState> g_interactions;
int g_nextUserId = 1;
int g_nextProductId = 101;
int g_nextInteractionId = 1;
bool g_seeded = false;

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

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
    std::string normalized = toLower(type);
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
    return std::to_string(userId) + ":" + std::to_string(itemId);
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

    auto productIt = g_products.find(entry.item_id);
    if (productIt != g_products.end()) {
        result["product"] = productToDict(productIt->second);
    } else {
        result["product"] = py::dict();
    }

    return result;
}

py::dict authResultToDict(const AuthResult& authResult) {
    py::dict result;
    result["success"] = authResult.success;
    result["error"] = authResult.error;
    result["user"] = userToDict(authResult.user);
    result["token"] = authResult.token;
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
    auto it = g_products.find(id);
    if (it == g_products.end()) {
        return nullptr;
    }
    return &it->second;
}

UserState* findUserById(int id) {
    auto it = g_users.find(id);
    if (it == g_users.end()) {
        return nullptr;
    }
    return &it->second;
}

UserState* findUserByEmail(const std::string& email) {
    std::string normalizedEmail = toLower(email);
    for (auto& [id, user] : g_users) {
        if (toLower(user.email) == normalizedEmail) {
            return &user;
        }
    }
    return nullptr;
}

void seedDefaults() {
    if (g_seeded) {
        return;
    }

    g_users.clear();
    g_products.clear();
    g_cart.clear();
    g_interactions.clear();

    g_users.emplace(1, UserState{1, "Mariam Saleh", "mariam@biteapple.test", "password123", "Electronics", "Cairo", 92.0, 8, "Normal"});
    g_users.emplace(2, UserState{2, "Omar Hassan", "omar@biteapple.test", "password123", "Fitness", "Alexandria", 76.0, 12, "Normal"});
    g_users.emplace(3, UserState{3, "Manager One", "manager@biteapple.test", "admin123", "Business", "Giza", 98.0, 20, "Normal"});

    g_products.emplace(101, ProductState{101, "AirPods Pro MaxCase", 2499.0, "Electronics", 94.0, 18, "", "Protective case for premium earbuds."});
    g_products.emplace(102, ProductState{102, "Smart Fitness Band", 1299.0, "Fitness", 87.0, 31, "", "Activity tracker with heart-rate monitoring."});
    g_products.emplace(103, ProductState{103, "Minimal Desk Lamp", 850.0, "Home", 78.0, 24, "", "LED desk lamp with warm light."});
    g_products.emplace(104, ProductState{104, "Campus Backpack", 1100.0, "Fashion", 83.0, 15, "", "Durable backpack for daily use."});
    g_products.emplace(105, ProductState{105, "Cold Brew Kit", 690.0, "Kitchen", 72.0, 22, "", "Simple glass cold brew maker."});
    g_products.emplace(106, ProductState{106, "Wireless Keyboard", 1650.0, "Electronics", 91.0, 12, "", "Low-profile wireless keyboard."});
    g_products.emplace(107, ProductState{107, "Yoga Recovery Mat", 780.0, "Fitness", 69.0, 27, "", "Supportive mat for recovery sessions."});
    g_products.emplace(108, ProductState{108, "Ceramic Dinner Set", 1850.0, "Home", 81.0, 9, "", "Six-piece dinner set."});
    g_products.emplace(109, ProductState{109, "Running Shoes", 1450.0, "Fitness", 74.0, 16, "", "Lightweight running shoes."});
    g_products.emplace(110, ProductState{110, "Espresso Grinder", 2100.0, "Kitchen", 79.0, 8, "", "Compact burr grinder."});

    g_nextUserId = 4;
    g_nextProductId = 111;
    g_nextInteractionId = 1;
    g_seeded = true;
}

std::vector<ProductState> allProductsSorted() {
    std::vector<ProductState> result;
    result.reserve(g_products.size());
    for (const auto& [id, product] : g_products) {
        result.push_back(product);
    }

    std::sort(result.begin(), result.end(), [](const ProductState& left, const ProductState& right) {
        if (left.popularity_score == right.popularity_score) {
            return left.id < right.id;
        }
        return left.popularity_score > right.popularity_score;
    });
    return result;
}

std::vector<ProductState> filterProductsByCategory(const std::string& category) {
    std::vector<ProductState> result;
    std::string normalized = toLower(category);
    for (const auto& [id, product] : g_products) {
        if (normalized.empty() || toLower(product.category) == normalized) {
            result.push_back(product);
        }
    }
    std::sort(result.begin(), result.end(), [](const ProductState& left, const ProductState& right) {
        if (left.popularity_score == right.popularity_score) {
            return left.id < right.id;
        }
        return left.popularity_score > right.popularity_score;
    });
    return result;
}

void updateUserActivity(UserState& user, const std::string& type) {
    user.activity_score += interactionWeight(type);
    user.level = userLevelFromScore(user.activity_score);
}

void updateProductPopularity(ProductState& product, const std::string& type) {
    product.popularity_score += interactionWeight(type);
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

    if (!user) {
        throw std::runtime_error("User not found");
    }
    if (!product) {
        throw std::runtime_error("Product not found");
    }
    if (canonicalType.empty()) {
        throw std::runtime_error("Unsupported interaction type");
    }

    InteractionState interaction = appendInteraction(userId, itemId, canonicalType);
    updateUserActivity(*user, canonicalType);
    updateProductPopularity(*product, canonicalType);
    return interaction;
}

std::vector<py::dict> trendingFromProducts(const std::vector<ProductState>& products, int limit) {
    std::vector<Item> items;
    items.reserve(products.size());
    for (const auto& product : products) {
        items.emplace_back(std::to_string(product.id), product.name, product.price, product.category, static_cast<int>(product.popularity_score), product.stock);
    }

    Heap heap(static_cast<int>(items.size()) + 1);
    for (const Item& item : items) {
        heap.insert(item);
    }

    std::vector<py::dict> result;
    int safeLimit = std::max(0, limit);
    for (int i = 0; i < safeLimit && !heap.isEmpty(); ++i) {
        Item item = heap.extractMax();
        int productId = 0;
        try {
            productId = std::stoi(item.getID());
        } catch (...) {
            productId = 0;
        }

        auto productIt = g_products.find(productId);
        if (productIt != g_products.end()) {
            result.push_back(productToDict(productIt->second));
        }
    }
    return result;
}

std::vector<py::dict> relatedProductsFromProducts(const std::vector<ProductState>& products, int itemId, int limit) {
    std::vector<Item> items;
    items.reserve(products.size());
    for (const auto& product : products) {
        items.emplace_back(std::to_string(product.id), product.name, product.price, product.category, static_cast<int>(product.popularity_score), product.stock);
    }

    if (items.empty()) {
        return {};
    }

    Graph graph;
    graph.build(items.data(), static_cast<int>(items.size()));

    int count = 0;
    std::string* neighbors = graph.getNeighbors(std::to_string(itemId), count);
    if (neighbors == nullptr) {
        return {};
    }

    std::vector<py::dict> result;
    int safeLimit = std::max(0, limit);
    for (int i = 0; i < count && static_cast<int>(result.size()) < safeLimit; ++i) {
        int neighborId = 0;
        try {
            neighborId = std::stoi(neighbors[i]);
        } catch (...) {
            continue;
        }

        auto productIt = g_products.find(neighborId);
        if (productIt == g_products.end()) {
            continue;
        }

        py::dict payload = productToDict(productIt->second);
        payload["similarity_score"] = 0.87;
        result.push_back(payload);
    }
    return result;
}

std::vector<py::dict> recommendationsFromProducts(
    const std::vector<ProductState>& products,
    const std::vector<InteractionState>& interactions,
    const UserState& user,
    int limit
) {
    if (products.empty()) {
        return {};
    }

    std::vector<Item> items;
    items.reserve(products.size());
    for (const auto& product : products) {
        items.emplace_back(std::to_string(product.id), product.name, product.price, product.category, static_cast<int>(product.popularity_score), product.stock);
    }

    Graph graph;
    graph.build(items.data(), static_cast<int>(items.size()));

    std::vector<std::string> interactedIds;
    for (const auto& interaction : interactions) {
        if (interaction.user_id == user.id) {
            std::string itemId = std::to_string(interaction.item_id);
            if (std::find(interactedIds.begin(), interactedIds.end(), itemId) == interactedIds.end()) {
                interactedIds.push_back(itemId);
            }
        }
    }

    std::vector<std::pair<py::dict, double>> scored;
    for (const auto& product : products) {
        if (product.stock <= 0) {
            continue;
        }

        double score = product.popularity_score;
        if (toLower(product.category) == toLower(user.category)) {
            score += 15.0;
        }

        for (const std::string& interactedId : interactedIds) {
            int count = 0;
            std::string* neighbors = graph.getNeighbors(interactedId, count);
            if (neighbors == nullptr) {
                continue;
            }
            for (int i = 0; i < count; ++i) {
                if (neighbors[i] == std::to_string(product.id)) {
                    score += 10.0;
                    break;
                }
            }
        }

        for (const auto& interaction : interactions) {
            if (interaction.user_id == user.id && interaction.item_id == product.id) {
                score += static_cast<double>(interactionWeight(interaction.type));
            }
        }

        py::dict payload = productToDict(product);
        payload["recommendation_score"] = score;

        std::string reason = "Because it matches your interests";
        if (toLower(product.category) == toLower(user.category)) {
            reason = "Because you interacted with " + product.category + " products";
        }
        payload["reason"] = reason;
        scored.emplace_back(payload, score);
    }

    std::sort(scored.begin(), scored.end(), [](const auto& left, const auto& right) {
        if (left.second == right.second) {
            auto leftId = py::cast<int>(left.first["id"]);
            auto rightId = py::cast<int>(right.first["id"]);
            return leftId < rightId;
        }
        return left.second > right.second;
    });

    std::vector<py::dict> result;
    int safeLimit = std::max(0, limit);
    for (int i = 0; i < safeLimit && i < static_cast<int>(scored.size()); ++i) {
        result.push_back(scored[i].first);
    }
    return result;
}

std::vector<py::dict> recentInteractionsFromList(const std::vector<InteractionState>& interactions, int limit) {
    std::vector<py::dict> result;
    int safeLimit = std::max(0, limit);
    for (auto it = interactions.rbegin(); it != interactions.rend() && static_cast<int>(result.size()) < safeLimit; ++it) {
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
    result.user = *user;
    result.token = "dev-token-" + std::to_string(user->id);
    return authResultToDict(result);
}

py::dict create_user(const py::dict& input) {
    seedDefaults();

    std::string name = py::cast<std::string>(input["name"]);
    std::string email = py::cast<std::string>(input["email"]);
    std::string password = py::cast<std::string>(input["password"]);
    std::string category = input.contains("category") ? py::cast<std::string>(input["category"]) : "Electronics";
    std::string region = input.contains("region") ? py::cast<std::string>(input["region"]) : "Cairo";

    if (findUserByEmail(email)) {
        throw std::runtime_error("Email is already registered");
    }

    UserState user;
    user.id = g_nextUserId++;
    user.name = name;
    user.email = email;
    user.password = password;
    user.category = category;
    user.region = region;
    user.score = 50.0;
    user.activity_score = 0;
    user.level = "Normal";
    g_users.emplace(user.id, user);

    AuthResult result;
    result.success = true;
    result.user = user;
    result.token = "dev-token-" + std::to_string(user.id);
    return authResultToDict(result);
}

py::dict get_user_by_id(int user_id) {
    seedDefaults();
    UserState* user = findUserById(user_id);
    if (!user) {
        throw std::runtime_error("User not found");
    }
    return userToDict(*user);
}

std::vector<py::dict> list_products(const std::string& category) {
    seedDefaults();
    std::vector<py::dict> result;
    for (const auto& product : filterProductsByCategory(category)) {
        result.push_back(productToDict(product));
    }
    return result;
}

py::dict get_product_by_id(int item_id) {
    seedDefaults();
    ProductState* product = findProduct(item_id);
    if (!product) {
        throw std::runtime_error("Product not found");
    }
    return productToDict(*product);
}

py::dict create_product(const py::dict& input) {
    seedDefaults();

    ProductState product;
    product.id = g_nextProductId++;
    product.name = py::cast<std::string>(input["name"]);
    product.price = py::cast<double>(input["price"]);
    product.category = py::cast<std::string>(input["category"]);
    product.stock = input.contains("stock") ? py::cast<int>(input["stock"]) : 0;
    product.image = input.contains("image") ? py::cast<std::string>(input["image"]) : "";
    product.description = input.contains("description") ? py::cast<std::string>(input["description"]) : "";
    product.popularity_score = input.contains("popularity_score") ? py::cast<double>(input["popularity_score"]) : 40.0;
    g_products.emplace(product.id, product);

    return productToDict(product);
}

py::dict update_product(int item_id, const py::dict& input) {
    seedDefaults();
    ProductState* product = findProduct(item_id);
    if (!product) {
        throw std::runtime_error("Product not found");
    }

    product->name = py::cast<std::string>(input["name"]);
    product->price = py::cast<double>(input["price"]);
    product->category = py::cast<std::string>(input["category"]);
    if (input.contains("stock")) {
        product->stock = py::cast<int>(input["stock"]);
    }
    if (input.contains("image")) {
        product->image = py::cast<std::string>(input["image"]);
    }
    if (input.contains("description")) {
        product->description = py::cast<std::string>(input["description"]);
    }
    if (input.contains("popularity_score")) {
        product->popularity_score = py::cast<double>(input["popularity_score"]);
    }

    return productToDict(*product);
}

bool delete_product(int item_id) {
    seedDefaults();
    auto productIt = g_products.find(item_id);
    if (productIt == g_products.end()) {
        return false;
    }

    g_products.erase(productIt);

    std::vector<std::string> keysToRemove;
    for (const auto& [key, entry] : g_cart) {
        if (entry.item_id == item_id) {
            keysToRemove.push_back(key);
        }
    }
    for (const std::string& key : keysToRemove) {
        g_cart.erase(key);
    }

    return true;
}

std::vector<py::dict> get_cart(int user_id) {
    seedDefaults();
    std::vector<py::dict> result;
    for (const auto& [key, entry] : g_cart) {
        if (entry.user_id == user_id) {
            result.push_back(cartItemToDict(entry));
        }
    }
    std::sort(result.begin(), result.end(), [](const py::dict& left, const py::dict& right) {
        return py::cast<int>(left["item_id"]) < py::cast<int>(right["item_id"]);
    });
    return result;
}

bool add_to_cart(int user_id, int item_id, int quantity) {
    seedDefaults();
    if (quantity < 1) {
        throw std::runtime_error("Quantity must be at least 1");
    }
    if (!findUserById(user_id)) {
        throw std::runtime_error("User not found");
    }
    if (!findProduct(item_id)) {
        throw std::runtime_error("Product not found");
    }

    std::string key = cartKey(user_id, item_id);
    auto it = g_cart.find(key);
    if (it == g_cart.end()) {
        g_cart.emplace(key, CartEntry{user_id, item_id, quantity});
    } else {
        it->second.quantity += quantity;
    }

    recordInteractionInternal(user_id, item_id, "cart");
    return true;
}

bool remove_from_cart(int user_id, int item_id) {
    seedDefaults();
    std::string key = cartKey(user_id, item_id);
    auto it = g_cart.find(key);
    if (it == g_cart.end()) {
        return false;
    }
    g_cart.erase(it);
    return true;
}

py::dict checkout(int user_id) {
    seedDefaults();
    if (!findUserById(user_id)) {
        throw std::runtime_error("User not found");
    }

    std::vector<CartEntry> entries;
    for (const auto& [key, entry] : g_cart) {
        if (entry.user_id == user_id) {
            entries.push_back(entry);
        }
    }

    if (entries.empty()) {
        throw std::runtime_error("Cart is empty");
    }

    int purchased = 0;
    for (const CartEntry& entry : entries) {
        ProductState* product = findProduct(entry.item_id);
        if (!product) {
            continue;
        }

        int quantity = std::max(1, entry.quantity);
        if (product->stock >= quantity) {
            product->stock -= quantity;
        } else {
            product->stock = 0;
        }

        for (int i = 0; i < quantity; ++i) {
            recordInteractionInternal(user_id, entry.item_id, "purchase");
        }

        g_cart.erase(cartKey(entry.user_id, entry.item_id));
        purchased += quantity;
    }

    CheckoutResult result;
    result.success = true;
    result.purchased = purchased;
    return checkoutResultToDict(result);
}

py::dict get_dashboard_stats() {
    seedDefaults();
    DashboardStats stats;
    stats.totalProducts = static_cast<int>(g_products.size());
    stats.totalUsers = static_cast<int>(g_users.size());
    stats.totalInteractions = static_cast<int>(g_interactions.size());

    std::unordered_map<std::string, double> categoryTotals;
    for (const auto& [id, product] : g_products) {
        categoryTotals[product.category] += product.popularity_score;
    }

    double bestScore = -1.0;
    for (const auto& [category, total] : categoryTotals) {
        if (total > bestScore) {
            bestScore = total;
            stats.mostPopularCategory = category;
        }
    }

    return dashboardStatsToDict(stats);
}

py::dict record_interaction(int user_id, int item_id, const std::string& interaction_type) {
    InteractionState interaction = recordInteractionInternal(user_id, item_id, interaction_type);
    return interactionToDict(interaction);
}

std::vector<py::dict> get_recent_interactions(int limit) {
    seedDefaults();
    std::vector<InteractionState> ordered = g_interactions;
    std::reverse(ordered.begin(), ordered.end());
    return recentInteractionsFromList(ordered, limit);
}

std::vector<py::dict> get_recommendations(int user_id, int limit) {
    seedDefaults();
    UserState* user = findUserById(user_id);
    if (!user) {
        return trendingFromProducts(allProductsSorted(), limit);
    }

    std::vector<InteractionState> userInteractions;
    for (const auto& interaction : g_interactions) {
        if (interaction.user_id == user_id) {
            userInteractions.push_back(interaction);
        }
    }

    return recommendationsFromProducts(allProductsSorted(), userInteractions, *user, limit);
}

std::vector<py::dict> get_related_products(int item_id, int limit) {
    seedDefaults();
    ProductState* product = findProduct(item_id);
    if (!product) {
        return {};
    }

    std::vector<ProductState> products = allProductsSorted();
    auto related = relatedProductsFromProducts(products, item_id, limit);
    if (!related.empty()) {
        return related;
    }

    std::vector<py::dict> fallback;
    for (const auto& candidate : products) {
        if (candidate.id == item_id) {
            continue;
        }
        if (toLower(candidate.category) == toLower(product->category)) {
            py::dict payload = productToDict(candidate);
            payload["similarity_score"] = 0.75;
            fallback.push_back(payload);
        }
    }

    std::sort(fallback.begin(), fallback.end(), [](const py::dict& left, const py::dict& right) {
        double leftScore = py::cast<double>(left["similarity_score"]);
        double rightScore = py::cast<double>(right["similarity_score"]);
        if (leftScore == rightScore) {
            return py::cast<int>(left["id"]) < py::cast<int>(right["id"]);
        }
        return leftScore > rightScore;
    });

    if (static_cast<int>(fallback.size()) > limit) {
        fallback.resize(std::max(0, limit));
    }
    return fallback;
}

std::vector<py::dict> get_trending(int limit) {
    seedDefaults();
    return trendingFromProducts(allProductsSorted(), limit);
}

// ---------------------------------------------------------------------------
// Legacy compatibility overloads for current Python wrappers
// ---------------------------------------------------------------------------

std::vector<py::dict> rank_top_products(const std::vector<py::dict>& products, int limit) {
    std::vector<ProductState> items;
    items.reserve(products.size());
    for (const py::dict& product : products) {
        ProductState item;
        item.id = py::cast<int>(product["id"]);
        item.name = py::cast<std::string>(product["name"]);
        item.price = py::cast<double>(product["price"]);
        item.category = py::cast<std::string>(product["category"]);
        item.popularity_score = py::cast<double>(product["popularity_score"]);
        item.stock = py::cast<int>(product["stock"]);
        if (product.contains("image")) {
            item.image = py::cast<std::string>(product["image"]);
        }
        if (product.contains("description")) {
            item.description = py::cast<std::string>(product["description"]);
        }
        items.push_back(item);
    }
    return trendingFromProducts(items, limit);
}

std::vector<py::dict> get_related_products(const std::vector<py::dict>& products, int item_id, int limit) {
    std::vector<ProductState> items;
    items.reserve(products.size());
    for (const py::dict& product : products) {
        ProductState item;
        item.id = py::cast<int>(product["id"]);
        item.name = py::cast<std::string>(product["name"]);
        item.price = py::cast<double>(product["price"]);
        item.category = py::cast<std::string>(product["category"]);
        item.popularity_score = py::cast<double>(product["popularity_score"]);
        item.stock = py::cast<int>(product["stock"]);
        if (product.contains("image")) {
            item.image = py::cast<std::string>(product["image"]);
        }
        if (product.contains("description")) {
            item.description = py::cast<std::string>(product["description"]);
        }
        items.push_back(item);
    }
    return relatedProductsFromProducts(items, item_id, limit);
}

std::vector<py::dict> get_recent_interactions(const std::vector<py::dict>& interactions, int limit) {
    std::vector<InteractionState> parsed;
    parsed.reserve(interactions.size());
    for (const py::dict& interaction : interactions) {
        InteractionState item;
        item.id = py::cast<int>(interaction["id"]);
        item.user_id = py::cast<int>(interaction["user_id"]);
        item.item_id = py::cast<int>(interaction["item_id"]);
        item.type = interaction.contains("type") ? py::cast<std::string>(interaction["type"]) : "view";
        item.timestamp = interaction.contains("timestamp") ? py::cast<std::string>(interaction["timestamp"]) : nowIso8601();
        parsed.push_back(item);
    }
    std::reverse(parsed.begin(), parsed.end());
    return recentInteractionsFromList(parsed, limit);
}

std::vector<py::dict> get_recommendations(const std::vector<py::dict>& products,
                                          const std::vector<py::dict>& interactions,
                                          const py::dict& user,
                                          int limit) {
    std::vector<ProductState> parsedProducts;
    parsedProducts.reserve(products.size());
    for (const py::dict& product : products) {
        ProductState item;
        item.id = py::cast<int>(product["id"]);
        item.name = py::cast<std::string>(product["name"]);
        item.price = py::cast<double>(product["price"]);
        item.category = py::cast<std::string>(product["category"]);
        item.popularity_score = py::cast<double>(product["popularity_score"]);
        item.stock = py::cast<int>(product["stock"]);
        if (product.contains("image")) {
            item.image = py::cast<std::string>(product["image"]);
        }
        if (product.contains("description")) {
            item.description = py::cast<std::string>(product["description"]);
        }
        parsedProducts.push_back(item);
    }

    std::vector<InteractionState> parsedInteractions;
    parsedInteractions.reserve(interactions.size());
    for (const py::dict& interaction : interactions) {
        InteractionState item;
        item.id = py::cast<int>(interaction["id"]);
        item.user_id = py::cast<int>(interaction["user_id"]);
        item.item_id = py::cast<int>(interaction["item_id"]);
        item.type = interaction.contains("type") ? py::cast<std::string>(interaction["type"]) : "view";
        item.timestamp = interaction.contains("timestamp") ? py::cast<std::string>(interaction["timestamp"]) : nowIso8601();
        parsedInteractions.push_back(item);
    }

    UserState parsedUser;
    parsedUser.id = py::cast<int>(user["id"]);
    parsedUser.name = user.contains("name") ? py::cast<std::string>(user["name"]) : "";
    parsedUser.email = user.contains("email") ? py::cast<std::string>(user["email"]) : "";
    parsedUser.category = user.contains("category") ? py::cast<std::string>(user["category"]) : "";

    return recommendationsFromProducts(parsedProducts, parsedInteractions, parsedUser, limit);
}

std::vector<py::dict> score_recommendations(const std::vector<py::dict>& products,
                                           const std::vector<py::dict>& interactions,
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
    m.def("create_user", &create_user, py::arg("input"));
    m.def("get_user_by_id", &get_user_by_id, py::arg("user_id"));

    m.def("list_products", &list_products, py::arg("category") = "");
    m.def("get_product_by_id", &get_product_by_id, py::arg("item_id"));
    m.def("create_product", &create_product, py::arg("input"));
    m.def("update_product", &update_product, py::arg("item_id"), py::arg("input"));
    m.def("delete_product", &delete_product, py::arg("item_id"));
    m.def("get_cart", &get_cart, py::arg("user_id"));
    m.def("add_to_cart", &add_to_cart, py::arg("user_id"), py::arg("item_id"), py::arg("quantity") = 1);
    m.def("remove_from_cart", &remove_from_cart, py::arg("user_id"), py::arg("item_id"));
    m.def("checkout", &checkout, py::arg("user_id"));
    m.def("get_dashboard_stats", &get_dashboard_stats);

    m.def("record_interaction", &record_interaction, py::arg("user_id"), py::arg("item_id"), py::arg("interaction_type"));
    m.def("get_recent_interactions", py::overload_cast<int>(&get_recent_interactions), py::arg("limit") = 8);
    m.def("get_recent_interactions", py::overload_cast<const std::vector<py::dict>&, int>(&get_recent_interactions), py::arg("interactions"), py::arg("limit") = 8);

    m.def("get_recommendations", py::overload_cast<int, int>(&get_recommendations), py::arg("user_id"), py::arg("limit") = 4);
    m.def("get_recommendations", py::overload_cast<const std::vector<py::dict>&, const std::vector<py::dict>&, const py::dict&, int>(&get_recommendations), py::arg("products"), py::arg("interactions"), py::arg("user"), py::arg("limit") = 4);

    m.def("get_related_products", py::overload_cast<int, int>(&get_related_products), py::arg("item_id"), py::arg("limit") = 3);
    m.def("get_related_products", py::overload_cast<const std::vector<py::dict>&, int, int>(&get_related_products), py::arg("products"), py::arg("item_id"), py::arg("limit") = 3);

    m.def("get_trending", py::overload_cast<int>(&get_trending), py::arg("limit") = 8);
    m.def("get_trending", py::overload_cast<const std::vector<py::dict>&, int>(&rank_top_products), py::arg("products"), py::arg("limit") = 8);

    m.def("rank_top_products", &rank_top_products, py::arg("products"), py::arg("limit") = 5);
    m.def("score_recommendations", &score_recommendations, py::arg("products"), py::arg("interactions"), py::arg("user"), py::arg("limit") = 4);
}
