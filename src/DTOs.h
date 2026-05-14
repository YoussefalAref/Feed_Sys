#ifndef DTOS_H
#define DTOS_H

#include <string>
#include "Vector.h" // Since your DTOs will be stored in your custom Vector

// Forward declare Item to avoid circular dependency with Item.h
class Item;

struct UserInput {
    std::string name;
    std::string email;
    std::string password;  
    std::string category;
    std::string region;
};

struct ProductInput {
    std::string name;
    double      price;
    std::string category;
    int         stock;
    std::string image;
    std::string description;
    double      popularity_score;
};

struct UserDTO {
    int         id;
    std::string name;
    std::string email;
    std::string category;
    double      score;
    std::string region;
    int         activity_score;
    std::string level;
};

struct AuthResult {
    bool        success;
    std::string error;      
    UserDTO     user;
    std::string token;      
};

struct ProductDTO {
    int         id;
    std::string name;
    double      price;
    std::string category;
    double      popularity_score;
    int         stock;
    std::string image;
    std::string description;
};

struct CartItemDTO {
    int        user_id;
    int        item_id;
    int        quantity;
    ProductDTO product;
};

struct CheckoutResult {
    bool success;
    int  purchased;   // number of distinct cart rows converted
};

struct DashboardStats {
    int         totalProducts;
    int         totalUsers;
    int         totalInteractions;
    std::string mostPopularCategory;
};

struct InteractionDTO {
    int         id;
    int         user_id;
    int         item_id;
    std::string type;       
    std::string timestamp;  
};

struct RecommendationDTO {
    ProductDTO  product;
    double      recommendation_score;
    std::string reason;
};

struct RelatedProductDTO {
    ProductDTO product;
    double     similarity_score;
};

#endif