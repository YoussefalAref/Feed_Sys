#ifndef RECOMMENDATION_ENGINE_H
#define RECOMMENDATION_ENGINE_H

#include "Item.h"
#include "Interaction.h"
#include "Graph.h"
#include <string>
using namespace std;

#define MAX_ITEMS 1000
#define MAX_CANDIDATES 500

class RecommendationEngine {
private:
    Graph graph;

    Item items[MAX_ITEMS];
    int itemCount;

    Interaction* interactions;
    int interactionCount;

public:
    RecommendationEngine();

    void loadData();

    int getCandidates(int userId, Item candidates[]);
    int scoreItems(int userId, Item candidates[], int count, double scores[]);

private:
    bool exists(Item arr[], int count, string itemId);

    double getSimilarity(int userId, string itemId);
    double getCategoryScore(int userId, Item& item);
    double getRecency(int userId, string itemId);
};

#endif