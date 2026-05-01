#include "Graph.h"

Graph::Graph() {
    size = 0;
}

int Graph::findIndex(string itemId) {
    for (int i = 0; i < size; i++) {
        if (itemIDs[i] == itemId)
            return i;
    }
    return -1;
}

void Graph::build(Item items[], int itemCount) {
    size = itemCount;

    for (int i = 0; i < itemCount; i++) {
        itemIDs[i] = items[i].getID();
        neighborCount[i] = 0;

        for (int j = 0; j < itemCount; j++) {
            if (i == j) continue;

            if (items[i].getCategory() == items[j].getCategory()) {
                if (neighborCount[i] < MAX_NEIGHBORS) {
                    neighbors[i][neighborCount[i]++] = items[j].getID();
                }
            }
        }
    }
}

string* Graph::getNeighbors(string itemId, int& count) {
    int index = findIndex(itemId);

    if (index == -1) {
        count = 0;
        return NULL;
    }

    count = neighborCount[index];
    return neighbors[index];
}