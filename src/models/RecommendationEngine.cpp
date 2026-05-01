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

bool RecommendationEngine::exists(Item arr[], int count, string itemId) {
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

        if (interactions[i].getUserId() == userId) {

            string itemId = interactions[i].getItemId();

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

        if (interactions[i].getUserId() == userId) {

            int neighborCount;
            string* neighbors =
                graph.getNeighbors(interactions[i].getItemId(), neighborCount);

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
        if (interactions[i].getUserId() == userId &&
            interactions[i].getItemId() == itemId) {
            return 1.0;
        }
    }

    return 0.0;
}