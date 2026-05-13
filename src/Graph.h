#ifndef GRAPH_H
#define GRAPH_H

#include "Item.h"
#include <string>
using namespace std;

#define MAX_ITEMS 1000
#define MAX_NEIGHBORS 20

class Graph {
private:
    string itemIDs[MAX_ITEMS];
    string neighbors[MAX_ITEMS][MAX_NEIGHBORS];
    int neighborCount[MAX_ITEMS];
    int size;

public:
    Graph();

    void build(Item items[], int itemCount);

    int findIndex(string itemId);

    string* getNeighbors(string itemId, int& count);
};

#endif