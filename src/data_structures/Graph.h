#ifndef GRAPH_H
#define GRAPH_H

/*
 * ============================================================
 *  FILE: src/data_structures/Graph.h
 *  OWNER: Developer 4 (Graph — Similarity Engine)
 * ============================================================
 *
 * PURPOSE
 * -------
 * The Graph models relationships between items.
 * If two items are "similar" (bought together, in the same category,
 * or viewed together), they are connected by an edge.
 *
 * Example:
 *   "Laptop"   — "Mouse"
 *   "Laptop"   — "Keyboard"
 *   "Phone"    — "Phone Case"
 *   "Phone"    — "Charger"
 *
 * The Graph is used by the recommendation engine to say:
 *   "You looked at item A → you might also like A's neighbours."
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: What is a Graph?
 *
 *   A graph is a collection of NODES (vertices) connected by EDGES.
 *
 *   Types of graphs:
 *     Directed   → edges have a direction (A → B, but not B → A)
 *     Undirected → edges go both ways   (A — B means A↔B)
 *
 *   For item similarity we use an UNDIRECTED graph (if Laptop is
 *   similar to Mouse, then Mouse is similar to Laptop).
 *
 *   Weighted graph → each edge has a weight (e.g. similarityScore).
 *   Unweighted     → all edges are equal.
 *
 *   For Milestone 1, start with UNWEIGHTED UNDIRECTED.
 *   Weights are an OPTIONAL ADVANCED feature (see below).
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: Adjacency List vs Adjacency Matrix
 *
 *   Adjacency Matrix:
 *     A 2D array where matrix[i][j] = 1 if there is an edge.
 *     Space: O(V²)  — wastes memory when edges are few (sparse graph).
 *     Lookup: O(1)  — fast to check if edge exists.
 *
 *   Adjacency List (RECOMMENDED for this project):
 *     Each node stores a LINKED LIST of its neighbours.
 *     Space: O(V + E)  — efficient for sparse graphs.
 *     Lookup: O(degree) — traverse the neighbour list.
 *
 *   Use adjacency list because in e-commerce, most items are
 *   only similar to a FEW other items (sparse graph).
 *
 * ─────────────────────────────────────────────────────────────
 *
 * NODE REPRESENTATION
 * -------------------
 *   In this graph, each NODE represents an item identified by its itemID.
 *   Nodes are NOT Item objects — just integer IDs.
 *   The HashMap holds the full Item data; the Graph holds only IDs + edges.
 *
 * INTERNAL STRUCTURE (Adjacency List)
 * -------------------------------------
 *   Step 1 — Define an EdgeNode struct (one entry in a neighbour list):
 *     EdgeNode:
 *       int       neighbourID    ← the itemID of the connected item
 *       double    weight         ← similarity score (use 1.0 if unweighted)
 *       EdgeNode* next           ← pointer to the next neighbour in the list
 *
 *   Step 2 — Define a GraphNode struct (one vertex / item in the graph):
 *     GraphNode:
 *       int        itemID        ← the item this node represents
 *       EdgeNode*  neighbours    ← head of the neighbour linked list
 *       GraphNode* next          ← pointer to the next GraphNode (for the node list)
 *
 *   Step 3 — Define the Graph struct/class:
 *     Fields:
 *       GraphNode* head          ← head of the list of all graph nodes
 *       int        nodeCount     ← total number of nodes in the graph
 *       int        edgeCount     ← total number of edges
 *
 * METHODS YOU NEED TO IMPLEMENT
 * --------------------------------
 *
 *   Graph()
 *     Constructor.
 *     Set head = nullptr, nodeCount = 0, edgeCount = 0.
 *
 *   ~Graph()
 *     Destructor.
 *     Free all EdgeNodes for each GraphNode, then free all GraphNodes.
 *     Prevents memory leaks.
 *
 *   void addNode(int itemID)
 *     Add a new vertex (item) to the graph.
 *     Steps:
 *       1. Check if the itemID already exists — if yes, do nothing.
 *       2. Create a new GraphNode with itemID and append it to the list.
 *       3. Increment nodeCount.
 *
 *   void addEdge(int itemA, int itemB, double weight = 1.0)
 *     Add an undirected edge between itemA and itemB.
 *     Steps:
 *       1. Make sure both itemA and itemB are already in the graph.
 *          (If not, call addNode() for the missing one.)
 *       2. Add itemB to itemA's neighbour list (with the given weight).
 *       3. Add itemA to itemB's neighbour list (because undirected).
 *       4. Increment edgeCount.
 *     NOTE: Check for duplicate edges — don't add the same edge twice.
 *
 *   GraphNode* findNode(int itemID)
 *     PRIVATE helper.
 *     Traverse the node list to find and return the GraphNode
 *     with the matching itemID.
 *     Return nullptr if not found.
 *
 *   void getNeighbours(int itemID)
 *     Print all neighbours of the item with the given ID.
 *     This is the key method used by the recommendation engine.
 *     Returns (or prints) the IDs of all items directly connected to itemID.
 *
 *   bool hasEdge(int itemA, int itemB)
 *     Returns true if there is an edge between itemA and itemB.
 *     Useful for checking before adding a duplicate edge.
 *
 *   void display()
 *     Print the entire adjacency list — all nodes and their neighbours.
 *     Example output:
 *       1 (Laptop)  → 2 → 3
 *       2 (Mouse)   → 1
 *       3 (Keyboard)→ 1
 *
 *   int getNodeCount()   — returns nodeCount
 *   int getEdgeCount()   — returns edgeCount
 *
 * OPTIONAL ADVANCED FEATURE: Weighted Edges
 * ------------------------------------------
 *   A weight on an edge represents HOW similar two items are.
 *   You can compute the weight based on:
 *     • How often they are viewed together
 *     • Whether they are in the same category
 *     • Whether they are commonly purchased together
 *   Store the weight in EdgeNode.weight.
 *   When building recommendations, prefer neighbours with HIGHER weight.
 *
 * GRAPH TRAVERSAL (NOT required for Milestone 1 — but good to know)
 * -------------------------------------------------------------------
 *   BFS (Breadth-First Search):
 *     Explore all immediate neighbours first, then their neighbours.
 *     Useful for finding items "2 hops away".
 *
 *   DFS (Depth-First Search):
 *     Go as deep as possible along one path before backtracking.
 *     Useful for exploring connected components.
 *
 * TESTING CHECKLIST (Developer 4)
 * --------------------------------
 *   [ ] Add 10 items (nodes) to the graph
 *   [ ] Connect them with edges (e.g. Laptop-Mouse, Laptop-Keyboard, etc.)
 *   [ ] Call getNeighbours(laptopID) — verify correct neighbours appear
 *   [ ] Call display() — verify the adjacency list looks correct
 *   [ ] Test hasEdge() for existing and non-existing edges
 *   [ ] Try adding a duplicate edge — make sure it's only stored once
 *
 * TODO (Developer 4)
 * ------------------
 *   [ ] Write the EdgeNode struct
 *   [ ] Write the GraphNode struct
 *   [ ] Write the Graph struct/class with head, nodeCount, edgeCount
 *   [ ] Implement constructor and destructor (no memory leaks!)
 *   [ ] Implement addNode() and findNode()
 *   [ ] Implement addEdge() (both directions) with duplicate check
 *   [ ] Implement getNeighbours(), hasEdge(), display()
 *   [ ] (Advanced/Optional) Add weighted edges and use them in scoring
 *   [ ] Test in main.cpp
 *   [ ] Document: explain adjacency list and how similarity works
 */

// ── YOUR CODE GOES BELOW THIS LINE ──────────────────────────


#endif // GRAPH_H
