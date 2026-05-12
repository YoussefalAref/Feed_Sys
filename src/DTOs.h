#ifndef DTOS_H
#define DTOS_H

#include <string>
#include "Vector.h" // Since your DTOs will be stored in your custom Vector
#include "Item.h"


// Cluster 2/3: Product Info
struct ProductDTO {
    int id;
    std::string name;
    double price;
    int popularity_score;
    std::string category;
};

// Cluster two: mirrors the fields that Item.h exposes via setters.
struct ProductInput {
    string name;
    string category;
    double price;
    int stock;
};

struct CartItemDTO {
    int item_id;
    string name;
    int quantity;
    double price;
};

struct CartItem {
    int item_id;
    int quantity;
};

// Cluster two: return value of checkout().
struct CheckoutResult {
    bool success;
    double total;
    string message;
};

// Cluster two: return value of get_dashboard_stats().
struct DashboardStats {
    int total_products;
    int total_users;
    int total_orders;
    double total_revenue;
};


// Cluster 3: Interaction Confirmation
struct InteractionDTO {
    int user_id;
    int item_id;
    std::string type;
    std::string timestamp;
};

// Cluster 3: Individual Recommendation
struct RecommendationDTO {
    ProductDTO product;
    double recommendation_score;
    std::string reason;
};

// Cluster 3: Related Product
struct RelatedProductDTO {
    ProductDTO product;
    double similarity_score;
};

#endif