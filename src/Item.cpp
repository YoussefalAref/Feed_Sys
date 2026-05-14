#include "Item.h"
#include "User.h"
#include "Vector.h"
#include <iostream>
#include <string>
using namespace std;

// Internal cart entry (not exposed via DTO)
struct CartItem {
    int item_id  = 0;
    int quantity = 0;
};

static Vector<ProductDTO> products;
static Vector<CartItem> userCarts[1000];
static int next_product_id = 1;
static int total_interactions = 0;

Item::Item(const std::string& id, const std::string& name, int popularityScore)
    : ID(id), name(name), price(0), category(""), popularityScore(popularityScore), stock(0) {}

Item::Item(string id, string n, double p, string c, int pS, int s){
    ID = id; name = n; price = p; category = c; popularityScore = pS; stock = s;
}

string Item::getID() const { return ID; }
string Item::getName() const { return name; }
double Item::getPrice() const { return price; }
string Item::getCategory() const { return category; }
int Item::getPopularityScore() const { return popularityScore; }
int Item::getStock() const { return stock; }

void Item::setID(string id) { ID = id; }
void Item::setName(string n) { name = n; }
void Item::setPrice(double p) { price = p; }
void Item::setCategory(string c) { category = c; }
void Item::setPopularityScore(int ps) { popularityScore = ps; }
void Item::setStock(int s) { stock = s; }
void Item::incrementPopularityScore(int num) { popularityScore += num; }
void Item::resetPopularity() { popularityScore = 0; }

void Item::displayItem() const {
    cout << "ID: " << ID << ", Name: " << name
         << ", Price: " << price << ", Category: " << category
         << ", Popularity: " << popularityScore << ", Stock: " << stock << endl;
}

Vector<ProductDTO> Item::list_products(const string& category) {
    Vector<ProductDTO> result;
    for (int i = 0; i < products.getSize(); i++) {
        ProductDTO p = products.get(i);
        if (category.empty() || p.category == category)
            result.push_back(p);
    }
    return result;
}

ProductDTO Item::get_product_by_id(int item_id) {
    for (int i = 0; i < products.getSize(); i++) {
        ProductDTO p = products.get(i);
        if (p.id == item_id) return p;
    }
    ProductDTO empty;
    empty.id = -1;
    return empty;
}

ProductDTO Item::create_product(const ProductInput& input) {
    ProductDTO product;
    product.id               = next_product_id++;
    product.name             = input.name;
    product.category         = input.category;
    product.price            = input.price;
    product.stock            = input.stock;
    product.image            = input.image;
    product.description      = input.description;
    product.popularity_score = input.popularity_score;
    products.push_back(product);
    return product;
}

ProductDTO Item::update_product(int item_id, const ProductInput& input) {
    for (int i = 0; i < products.getSize(); i++) {
        ProductDTO p = products.get(i);
        if (p.id == item_id) {
            p.name             = input.name;
            p.category         = input.category;
            p.price            = input.price;
            p.stock            = input.stock;
            p.image            = input.image;
            p.description      = input.description;
            p.popularity_score = input.popularity_score;
            products.set(i, p);
            return p;
        }
    }
    ProductDTO empty; empty.id = -1; return empty;
}

bool Item::delete_product(int item_id) {
    for (int i = 0; i < products.getSize(); i++) {
        if (products.get(i).id == item_id) {
            products.remove(i);
            return true;
        }
    }
    return false;
}

Vector<CartItemDTO> Item::get_cart(int user_id) {
    Vector<CartItemDTO> result;
    if (user_id < 0 || user_id >= 1000) return result;
    for (int i = 0; i < userCarts[user_id].getSize(); i++) {
        CartItem ci = userCarts[user_id].get(i);
        ProductDTO product = get_product_by_id(ci.item_id);
        if (product.id != -1) {
            CartItemDTO dto;
            dto.user_id  = user_id;
            dto.item_id  = ci.item_id;
            dto.quantity = ci.quantity;
            dto.product  = product;
            result.push_back(dto);
        }
    }
    return result;
}

bool Item::add_to_cart(int user_id, int item_id, int quantity) {
    if (user_id < 0 || user_id >= 1000) return false;
    ProductDTO product = get_product_by_id(item_id);
    if (product.id == -1 || product.stock < quantity) return false;

    for (int i = 0; i < userCarts[user_id].getSize(); i++) {
        CartItem ci = userCarts[user_id].get(i);
        if (ci.item_id == item_id) {
            ci.quantity += quantity;
            userCarts[user_id].set(i, ci);
            return true;
        }
    }
    CartItem newItem; newItem.item_id = item_id; newItem.quantity = quantity;
    userCarts[user_id].push_back(newItem);
    return true;
}

bool Item::remove_from_cart(int user_id, int item_id) {
    if (user_id < 0 || user_id >= 1000) return false;
    for (int i = 0; i < userCarts[user_id].getSize(); i++) {
        if (userCarts[user_id].get(i).item_id == item_id) {
            userCarts[user_id].remove(i);
            return true;
        }
    }
    return false;
}

CheckoutResult Item::checkout(int user_id) {
    CheckoutResult result;
    result.success   = false;
    result.purchased = 0;
    if (user_id < 0 || user_id >= 1000 || userCarts[user_id].isEmpty()) return result;

    // Verify stock
    for (int i = 0; i < userCarts[user_id].getSize(); i++) {
        CartItem ci = userCarts[user_id].get(i);
        ProductDTO p = get_product_by_id(ci.item_id);
        if (p.stock < ci.quantity) return result;
    }

    int purchased = 0;
    for (int i = 0; i < userCarts[user_id].getSize(); i++) {
        CartItem ci = userCarts[user_id].get(i);
        for (int j = 0; j < products.getSize(); j++) {
            ProductDTO p = products.get(j);
            if (p.id == ci.item_id) {
                p.stock -= ci.quantity;
                products.set(j, p);
                purchased += ci.quantity;
            }
        }
    }
    total_interactions += purchased;

    while (!userCarts[user_id].isEmpty()) userCarts[user_id].pop_back();

    result.success   = true;
    result.purchased = purchased;
    return result;
}

DashboardStats Item::get_dashboard_stats() {
    DashboardStats stats;
    stats.totalProducts          = products.getSize();
    stats.totalUsers             = 0;
    stats.totalInteractions      = total_interactions;
    stats.mostPopularCategory    = "";
    return stats;
}
