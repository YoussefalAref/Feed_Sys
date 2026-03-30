# Graph Similarity Engine

## Why a Graph Is Used

A recommendation problem is a relationship problem: items are connected to other items based on user behavior or content signals.
A graph models this directly.

- Node: one item ID
- Edge: a similarity relation between two items
- Weight: strength of similarity (optional advanced mode)

This structure is a good fit because most products are related to only a small subset of products, so an adjacency list gives memory usage close to O(V + E) instead of O(V^2).

## How Similarity Works

For an item A, recommendations come from A's neighbors.

1. User interacts with item A.
2. Fetch neighbors of A from the graph.
3. Rank neighbors by edge weight (if weights are enabled) and/or product popularity.
4. Return top candidates as "You might also like".

Example:

- Edge Laptop - Mouse with weight 0.95
- Edge Laptop - Keyboard with weight 0.80

If user views Laptop, Mouse is prioritized over Keyboard.

## Implementation Notes

- Undirected graph: if A is similar to B, then B is similar to A.
- Duplicate edges are prevented.
- Re-adding an existing edge updates its weight instead of creating another edge.
- API provided in Graph:
  - addEdge(itemA, itemB, weight)
  - getNeighbors(item)
  - getNeighborsWithWeights(item)
  - hasEdge(itemA, itemB)
