#ifndef DTOS_H
#define DTOS_H

#include <string>
#include "Vector.h" // Since your DTOs will be stored in your custom Vector

// Cluster 2/3: Product Info
struct ProductDTO {
    int id;
    std::string name;
    double price;
    int popularity_score;
    std::string category;
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