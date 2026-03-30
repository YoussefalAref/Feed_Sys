#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <map>
#include <utility>
#include <vector>

class Graph {
private:
	struct EdgeNode {
		int neighbourID;
		double weight;
		EdgeNode* next;

		EdgeNode(int id, double w) : neighbourID(id), weight(w), next(nullptr) {}
	};

	// itemID -> head of neighbour linked list
	std::map<int, EdgeNode*> adjacency;
	int nodeCount;
	int edgeCount;

	EdgeNode* findEdge(int fromItemID, int toItemID) const;
	void deleteEdgeList(EdgeNode* head);

public:
	Graph();
	~Graph();

	void addNode(int itemID);
	void addEdge(int itemA, int itemB, double weight = 1.0);

	bool hasEdge(int itemA, int itemB) const;

	// American spelling (requested API)
	std::vector<int> getNeighbors(int itemID) const;
	std::vector<std::pair<int, double>> getNeighborsWithWeights(int itemID) const;

	// British spelling compatibility with existing docs/main comments
	std::vector<int> getNeighbours(int itemID) const;

	void display(std::ostream& out = std::cout) const;

	int getNodeCount() const;
	int getEdgeCount() const;
};


#endif // GRAPH_H
