#include "../data_structures/Graph.h"
#include "../data_structures/Heap.h"
#include "../data_structures/Queue.h"
#include "../models/Interaction.h"
#include "../models/Item.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace {
int readInt(const py::dict& data, const char* key, int fallback = 0) {
    py::str pyKey(key);
    if (!data.contains(pyKey)) return fallback;

    py::handle value = data[pyKey];
    try {
        return py::cast<int>(value);
    } catch (const py::cast_error&) {
        try {
            return std::stoi(py::cast<std::string>(py::str(value)));
        } catch (...) {
            return fallback;
        }
    }
}

double readDouble(const py::dict& data, const char* key, double fallback = 0.0) {
    py::str pyKey(key);
    if (!data.contains(pyKey)) return fallback;

    try {
        return py::cast<double>(data[pyKey]);
    } catch (const py::cast_error&) {
        return fallback;
    }
}

std::string readString(const py::dict& data, const char* key, const std::string& fallback = "") {
    py::str pyKey(key);
    if (!data.contains(pyKey)) return fallback;
    return py::cast<std::string>(py::str(data[pyKey]));
}

int readPopularityScore(const py::dict& data) {
    py::str backendKey("popularity_score");
    if (data.contains(backendKey)) return readInt(data, "popularity_score");
    return readInt(data, "score");
}

Item dictToItem(const py::dict& data) {
    int id = readInt(data, "id");
    return Item(
        std::to_string(id),
        readString(data, "name"),
        readDouble(data, "price"),
        readString(data, "category"),
        readPopularityScore(data),
        readInt(data, "stock")
    );
}

int itemIdToInt(const std::string& itemId) {
    try {
        return std::stoi(itemId);
    } catch (...) {
        return 0;
    }
}

py::dict itemToDict(const Item& item) {
    py::dict data;
    data["id"] = itemIdToInt(item.getID());
    data["name"] = item.getName();
    data["price"] = item.getPrice();
    data["category"] = item.getCategory();
    data["popularity_score"] = item.getPopularityScore();
    data["stock"] = item.getStock();
    return data;
}

std::vector<Item> dictsToItems(const std::vector<py::dict>& products) {
    std::vector<Item> items;
    items.reserve(products.size());
    for (const py::dict& product : products) {
        items.push_back(dictToItem(product));
    }
    return items;
}

bool hasNeighbor(Graph& graph, const std::string& sourceId, const std::string& targetId) {
    int count = 0;
    std::string* neighbors = graph.getNeighbors(sourceId, count);
    if (neighbors == nullptr) return false;

    for (int i = 0; i < count; ++i) {
        if (neighbors[i] == targetId) return true;
    }
    return false;
}

int interactionWeight(const std::string& type) {
    if (type == "view") return 1;
    if (type == "click") return 2;
    if (type == "cart" || type == "add_to_cart") return 5;
    if (type == "purchase") return 10;
    return 0;
}

std::vector<std::string> userInteractionItemIds(
    const std::vector<py::dict>& interactions,
    int userId
) {
    std::vector<std::string> itemIds;
    for (const py::dict& interaction : interactions) {
        if (readInt(interaction, "user_id") != userId) continue;

        std::string itemId = std::to_string(readInt(interaction, "item_id"));
        if (std::find(itemIds.begin(), itemIds.end(), itemId) == itemIds.end()) {
            itemIds.push_back(itemId);
        }
    }
    return itemIds;
}
}

std::vector<py::dict> rankTopProducts(const std::vector<py::dict>& products, int limit) {
    std::vector<Item> items = dictsToItems(products);
    Heap heap(static_cast<int>(items.size()));

    for (const Item& item : items) {
        heap.insert(item);
    }

    std::vector<py::dict> ranked;
    int safeLimit = std::max(0, limit);
    for (int i = 0; i < safeLimit && !heap.isEmpty(); ++i) {
        ranked.push_back(itemToDict(heap.extractMax()));
    }
    return ranked;
}

std::vector<py::dict> getRelatedProducts(
    const std::vector<py::dict>& products,
    int itemId,
    int limit
) {
    std::vector<Item> items = dictsToItems(products);
    if (items.empty()) return {};

    Graph graph;
    graph.build(items.data(), static_cast<int>(items.size()));

    int count = 0;
    std::string* neighbors = graph.getNeighbors(std::to_string(itemId), count);
    if (neighbors == nullptr) return {};

    std::vector<py::dict> related;
    int safeLimit = std::max(0, limit);

    for (int i = 0; i < count && static_cast<int>(related.size()) < safeLimit; ++i) {
        for (const Item& item : items) {
            if (item.getID() == neighbors[i]) {
                related.push_back(itemToDict(item));
                break;
            }
        }
    }
    return related;
}

std::vector<py::dict> getRecentInteractions(const std::vector<py::dict>& interactions, int limit) {
    std::vector<py::dict> recent;
    int safeLimit = std::max(0, limit);

    for (auto it = interactions.rbegin(); it != interactions.rend(); ++it) {
        if (static_cast<int>(recent.size()) >= safeLimit) break;
        recent.push_back(*it);
    }
    return recent;
}

std::vector<py::dict> scoreRecommendations(
    const std::vector<py::dict>& products,
    const std::vector<py::dict>& interactions,
    const py::dict& user,
    int limit
) {
    std::vector<Item> items = dictsToItems(products);
    if (items.empty()) return {};

    Graph graph;
    graph.build(items.data(), static_cast<int>(items.size()));

    int userId = readInt(user, "id");
    std::string preferredCategory = readString(user, "category");
    std::vector<std::string> interactedIds = userInteractionItemIds(interactions, userId);

    Heap candidates(static_cast<int>(items.size()));

    for (Item item : items) {
        int score = item.getPopularityScore();
        if (item.getCategory() == preferredCategory) {
            score += 15;
        }

        for (const std::string& interactedId : interactedIds) {
            if (hasNeighbor(graph, interactedId, item.getID())) {
                score += 10;
                break;
            }
        }

        for (const py::dict& interaction : interactions) {
            if (readInt(interaction, "user_id") == userId &&
                std::to_string(readInt(interaction, "item_id")) == item.getID()) {
                score += interactionWeight(readString(interaction, "type"));
            }
        }

        item.setPopularityScore(score);
        candidates.insert(item);
    }

    std::vector<py::dict> ranked;
    int safeLimit = std::max(0, limit);
    for (int i = 0; i < safeLimit && !candidates.isEmpty(); ++i) {
        ranked.push_back(itemToDict(candidates.extractMax()));
    }
    return ranked;
}

PYBIND11_MODULE(biteapple_core, m) {
    m.doc() = "BiteApple C++ ADS core bindings";

    m.def("rank_top_products", &rankTopProducts, py::arg("products"), py::arg("limit") = 5);
    m.def("get_related_products", &getRelatedProducts, py::arg("products"), py::arg("item_id"), py::arg("limit") = 3);
    m.def("get_recent_interactions", &getRecentInteractions, py::arg("interactions"), py::arg("limit") = 8);
    m.def(
        "score_recommendations",
        &scoreRecommendations,
        py::arg("products"),
        py::arg("interactions"),
        py::arg("user"),
        py::arg("limit") = 4
    );
}
