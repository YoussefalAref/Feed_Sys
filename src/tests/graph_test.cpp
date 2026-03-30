#include "../data_structures/Graph.h"

#include <cassert>
#include <iostream>
#include <vector>

static bool contains(const std::vector<int>& values, int target) {
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] == target) {
            return true;
        }
    }
    return false;
}

int main() {
    Graph graph;

    graph.addEdge(101, 102, 0.9); // Laptop - Mouse
    graph.addEdge(101, 103, 0.8); // Laptop - Keyboard
    graph.addEdge(102, 104, 0.7); // Mouse - Mousepad

    // Duplicate should not increase undirected edge count.
    graph.addEdge(101, 102, 0.95);

    assert(graph.getNodeCount() == 4);
    assert(graph.getEdgeCount() == 3);

    assert(graph.hasEdge(101, 102));
    assert(graph.hasEdge(102, 101));
    assert(!graph.hasEdge(101, 104));

    std::vector<int> neighbors = graph.getNeighbors(101);
    assert(neighbors.size() == 2);
    assert(contains(neighbors, 102));
    assert(contains(neighbors, 103));

    std::vector<std::pair<int, double>> weighted = graph.getNeighborsWithWeights(101);
    assert(weighted.size() == 2);

    std::cout << "Graph tests passed.\n";
    graph.display();

    return 0;
}
