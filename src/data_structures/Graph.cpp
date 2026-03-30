#include "Graph.h"

Graph::Graph() : nodeCount(0), edgeCount(0) {}

Graph::~Graph() {
    for (std::map<int, EdgeNode*>::iterator it = adjacency.begin(); it != adjacency.end(); ++it) {
        deleteEdgeList(it->second);
    }
}

void Graph::deleteEdgeList(EdgeNode* head) {
    while (head != nullptr) {
        EdgeNode* next = head->next;
        delete head;
        head = next;
    }
}

Graph::EdgeNode* Graph::findEdge(int fromItemID, int toItemID) const {
    std::map<int, EdgeNode*>::const_iterator it = adjacency.find(fromItemID);
    if (it == adjacency.end()) {
        return nullptr;
    }

    EdgeNode* current = it->second;
    while (current != nullptr) {
        if (current->neighbourID == toItemID) {
            return current;
        }
        current = current->next;
    }

    return nullptr;
}

void Graph::addNode(int itemID) {
    if (adjacency.find(itemID) != adjacency.end()) {
        return;
    }

    adjacency[itemID] = nullptr;
    nodeCount++;
}

void Graph::addEdge(int itemA, int itemB, double weight) {
    if (itemA == itemB) {
        return;
    }

    addNode(itemA);
    addNode(itemB);

    EdgeNode* existingAB = findEdge(itemA, itemB);
    EdgeNode* existingBA = findEdge(itemB, itemA);

    // Keep a single undirected edge; update weight if already present.
    if (existingAB != nullptr && existingBA != nullptr) {
        existingAB->weight = weight;
        existingBA->weight = weight;
        return;
    }

    EdgeNode* nodeB = new EdgeNode(itemB, weight);
    nodeB->next = adjacency[itemA];
    adjacency[itemA] = nodeB;

    EdgeNode* nodeA = new EdgeNode(itemA, weight);
    nodeA->next = adjacency[itemB];
    adjacency[itemB] = nodeA;

    edgeCount++;
}

bool Graph::hasEdge(int itemA, int itemB) const {
    return findEdge(itemA, itemB) != nullptr;
}

std::vector<int> Graph::getNeighbors(int itemID) const {
    std::vector<int> neighbours;

    std::map<int, EdgeNode*>::const_iterator it = adjacency.find(itemID);
    if (it == adjacency.end()) {
        return neighbours;
    }

    EdgeNode* current = it->second;
    while (current != nullptr) {
        neighbours.push_back(current->neighbourID);
        current = current->next;
    }

    return neighbours;
}

std::vector<std::pair<int, double>> Graph::getNeighborsWithWeights(int itemID) const {
    std::vector<std::pair<int, double>> neighbours;

    std::map<int, EdgeNode*>::const_iterator it = adjacency.find(itemID);
    if (it == adjacency.end()) {
        return neighbours;
    }

    EdgeNode* current = it->second;
    while (current != nullptr) {
        neighbours.push_back(std::make_pair(current->neighbourID, current->weight));
        current = current->next;
    }

    return neighbours;
}

std::vector<int> Graph::getNeighbours(int itemID) const {
    return getNeighbors(itemID);
}

void Graph::display(std::ostream& out) const {
    for (std::map<int, EdgeNode*>::const_iterator it = adjacency.begin(); it != adjacency.end(); ++it) {
        out << it->first << " -> ";

        EdgeNode* current = it->second;
        if (current == nullptr) {
            out << "(none)";
        }

        while (current != nullptr) {
            out << current->neighbourID << "(w=" << current->weight << ")";
            current = current->next;
            if (current != nullptr) {
                out << " -> ";
            }
        }

        out << '\n';
    }
}

int Graph::getNodeCount() const {
    return nodeCount;
}

int Graph::getEdgeCount() const {
    return edgeCount;
}
