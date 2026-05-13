#include "RecommendationEngine.h"
#include "User.h"
#include <iostream>
using namespace std;

// Assume these exist from other developers
extern Item* getAllItems(int& count);
extern Interaction* getAllInteractions(int& count);
extern User getUser(int userId);

RecommendationEngine::RecommendationEngine() {
    itemCount = 0;
    interactionCount = 0;
}

void RecommendationEngine::loadData() {

    Item* allItems = getAllItems(itemCount);
    for (int i = 0; i < itemCount; i++) {
        items[i] = allItems[i];
    }

    interactions = getAllInteractions(interactionCount);

    graph.build(items, itemCount);
}

bool RecommendationEngine::exists(Item arr[], int count, std::string itemId) {
    for (int i = 0; i < count; i++) {
        if (arr[i].getID() == itemId)
            return true;
    }
    return false;
}

int RecommendationEngine::getCandidates(int userId, Item candidates[]) {

    int count = 0;

    // 1. FROM USER INTERACTIONS
    for (int i = 0; i < interactionCount; i++) {

        if (interactions[i].getUserID() == userId) {

            std::string itemId = intToString(interactions[i].getItemID());

            if (!exists(candidates, count, itemId)) {

                for (int j = 0; j < itemCount; j++) {
                    if (items[j].getID() == itemId) {
                        candidates[count++] = items[j];
                        break;
                    }
                }
            }
        }
    }

    // 2. FROM GRAPH (SIMILAR ITEMS)
    for (int i = 0; i < count; i++) {

        int neighborCount;
        string* neighbors =
            graph.getNeighbors(candidates[i].getID(), neighborCount);

        for (int j = 0; j < neighborCount; j++) {

            if (!exists(candidates, count, neighbors[j])) {

                for (int k = 0; k < itemCount; k++) {
                    if (items[k].getID() == neighbors[j]) {
                        candidates[count++] = items[k];
                        break;
                    }
                }
            }
        }
    }

    // 3. CATEGORY-BASED
    string userCategory = getUser(userId).getCategory();

    for (int i = 0; i < itemCount; i++) {
        if (items[i].getCategory() == userCategory) {

            if (!exists(candidates, count, items[i].getID())) {
                candidates[count++] = items[i];
            }
        }
    }

    // 4. POPULAR ITEMS
    for (int i = 0; i < itemCount; i++) {
        if (items[i].getPopularityScore() > 80) {

            if (!exists(candidates, count, items[i].getID())) {
                candidates[count++] = items[i];
            }
        }
    }

    return count;
}

int RecommendationEngine::scoreItems(int userId, Item candidates[], int count, double scores[]) {

    for (int i = 0; i < count; i++) {

        double score = 0;

        score += 0.4 * getSimilarity(userId, candidates[i].getID());
        score += 0.3 * getCategoryScore(userId, candidates[i]);

        double popularity = candidates[i].getPopularityScore() / 100.0;
        score += 0.2 * popularity;

        score += 0.1 * getRecency(userId, candidates[i].getID());

        scores[i] = score;
    }

    return count;
}

double RecommendationEngine::getSimilarity(int userId, string itemId) {

    for (int i = 0; i < interactionCount; i++) {

        if (interactions[i].getUserID() == userId) {

            int neighborCount;
            string* neighbors =
                graph.getNeighbors(intToString(interactions[i].getItemID() ), neighborCount);

            for (int j = 0; j < neighborCount; j++) {
                if (neighbors[j] == itemId)
                    return 1.0;
            }
        }
    }

    return 0.0;
}

double RecommendationEngine::getCategoryScore(int userId, Item& item) {

    string userCategory = getUser(userId).getCategory();

    if (item.getCategory() == userCategory)
        return 1.0;

    return 0.0;
}

double RecommendationEngine::getRecency(int userId, string itemId) {

    for (int i = 0; i < interactionCount; i++) {
        if (interactions[i].getUserID() == userId && intToString(interactions[i].getItemID()) == itemId) {
            return 1.0;
        }
    }

    return 0.0;
}

Vector<RecommendationDTO> RecommendationEngine::get_recommendations(int user_id, int limit) {
    Item candidates[100]; // Array for existing logic
    int count = getCandidates(user_id, candidates);
    
    double scores[100];
    scoreItems(user_id, candidates, count, scores);

    // Manual Bubble Sort (Highest score first)
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (scores[j] < scores[j + 1]) {
                // Swap scores and items
                double tempS = scores[j]; scores[j] = scores[j+1]; scores[j+1] = tempS;
                Item tempI = candidates[j]; candidates[j] = candidates[j+1]; candidates[j+1] = tempI;
            }
        }
    }

    Vector<RecommendationDTO> results;
    for (int i = 0; i < count && i < limit; i++) {
        RecommendationDTO rec;
        rec.product.id = std::stoi(candidates[i].getID());
        rec.product.name = candidates[i].getName();
        rec.recommendation_score = scores[i];
        rec.reason = "Based on your interest in " + candidates[i].getCategory();
        results.push_back(rec);
    }
    return results;
}

Vector<RelatedProductDTO> RecommendationEngine::get_related_products(int item_id, int limit) {
    Vector<RelatedProductDTO> related;
    int neighborCount = 0;
    
    // Get neighbors from your custom Graph
    string* neighbors = graph.getNeighbors(intToString(item_id), neighborCount);

    for (int i = 0; i < neighborCount && i < limit; i++) {
        RelatedProductDTO rDTO;
        // Search for the neighbor item in your storage to get its name/price
        Item neighborItem = itemStore.search(neighbors[i]);
        
        rDTO.product.id = std::stoi(neighbors[i]);
        rDTO.product.name = neighborItem.getName();
        rDTO.similarity_score = 0.85; // Example constant or calculated weight
        
        related.push_back(rDTO);
    }
    return related;
}

// Inside RecommendationEngine.cpp

Vector<ProductDTO> RecommendationEngine::get_trending(int limit) {
    Vector<ProductDTO> trendingList;
    
    // 1. Get all items from your storage
    int totalItems = 0;
    Item* allItems = getAllItems(totalItems); // Your existing helper

    // 2. Manual Sort (since no STL std::sort)
    // We sort by popularity score in descending order
    for (int i = 0; i < totalItems - 1; i++) {
        for (int j = 0; j < totalItems - i - 1; j++) {
            if (allItems[j].getPopularityScore() < allItems[j + 1].getPopularityScore()) {
                Item temp = allItems[j];
                allItems[j] = allItems[j + 1];
                allItems[j + 1] = temp;
            }
        }
    }

    // 3. Convert the top 'limit' items to ProductDTOs
    for (int i = 0; i < totalItems && i < limit; i++) {
        ProductDTO dto;
        dto.id = std::stoi(allItems[i].getID());
        dto.name = allItems[i].getName();
        dto.popularity_score = allItems[i].getPopularityScore();
        // Add other required fields from your ProductDTO definition
        
        trendingList.push_back(dto);
    }

    return trendingList;
}