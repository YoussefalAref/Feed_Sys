/*
 * ============================================================
 *  FILE: src/main.cpp
 *  OWNER: Developer 5 (Integration + Models)
 * ============================================================
 *
 * PURPOSE
 * -------
 * This is the entry point of the program for Milestone 1.
 * Its job is to:
 *   1. Create sample data (users and items)
 *   2. Store them in the appropriate data structures
 *   3. Simulate user interactions
 *   4. Display rankings and recommendations
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: #include and compilation
 *
 *   #include "path/to/file.h" tells the compiler to copy the
 *   contents of that header file into this file before compiling.
 *
 *   You will need:
 *     #include "models/User.h"
 *     #include "models/Item.h"
 *     #include "models/Interaction.h"
 *     #include "data_structures/HashMap.h"
 *     #include "data_structures/Heap.h"
 *     #include "data_structures/Queue.h"
 *     #include "data_structures/Graph.h"
 *     #include <iostream>
 *     using namespace std;
 * ─────────────────────────────────────────────────────────────
 *
 * HOW TO COMPILE (from the src/ directory)
 * -----------------------------------------
 *   g++ -std=c++17 -Wall main.cpp -o feed_sys
 *   ./feed_sys
 *
 *   Or from the project root:
 *   g++ -std=c++17 -Wall src/main.cpp -o build/feed_sys
 *
 * ─────────────────────────────────────────────────────────────
 * DEMO SCENARIO TO SIMULATE
 * --------------------------
 *
 *   STEP 1 — Create Users
 *     Create at least 5 users and insert them into UserHashMap.
 *     Example:
 *       User alice(1, "alice", "alice@mail.com", "pass123")
 *       userMap.insert(1, alice)
 *
 *   STEP 2 — Create Items
 *     Create at least 10 items and insert them into ItemHashMap.
 *     Also insert each item into the global Heap.
 *     Example:
 *       Item laptop(101, "Laptop", "Electronics", 999.99, 0.0, 50)
 *       itemMap.insert(101, laptop)
 *       globalHeap.insert(laptop)
 *
 *   STEP 3 — Build Item Similarity Graph
 *     Add all items as nodes.
 *     Add edges between related items.
 *     Example:
 *       graph.addNode(101)   // Laptop
 *       graph.addNode(102)   // Mouse
 *       graph.addEdge(101, 102)   // Laptop — Mouse
 *       graph.addEdge(101, 103)   // Laptop — Keyboard
 *
 *   STEP 4 — Simulate Interactions
 *     Create Interaction objects and enqueue them.
 *     Example:
 *       Interaction i1(1, 101, VIEW)     // alice viewed Laptop
 *       interactionQueue.enqueue(i1)
 *       Interaction i2(1, 101, PURCHASE) // alice bought Laptop
 *       interactionQueue.enqueue(i2)
 *
 *   STEP 5 — Process the Queue
 *     Dequeue each interaction and update scores.
 *     Pseudocode:
 *       while (!interactionQueue.isEmpty()) {
 *           Interaction inter = interactionQueue.dequeue()
 *           // Find the item in itemMap
 *           // Update item.popularityScore based on inter.type
 *           // Find the user in userMap
 *           // Update user.activityScore based on inter.type
 *       }
 *
 *   STEP 6 — Rebuild the Heap (after score updates)
 *     After all interactions are processed, re-insert all items
 *     into a fresh heap so the rankings reflect the new scores.
 *
 *   STEP 7 — Display Results
 *     a) Print top 5 items (for new users):
 *          for (int i = 0; i < 5 && !heap.isEmpty(); i++)
 *              cout << heap.extractMax().name << endl
 *
 *     b) Print all user levels (Normal / Active / VIP):
 *          userMap.display()
 *
 *     c) Print item neighbours from the graph:
 *          graph.getNeighbours(101)  // Similar to Laptop
 *
 * ─────────────────────────────────────────────────────────────
 * IMPORTANT NOTES FOR DEVELOPER 5
 * ----------------------------------
 *   • You are responsible for INTEGRATING all four data structures.
 *   • Make sure each team member's module compiles correctly before
 *     you try to combine them.
 *   • Start simple: get the HashMap working first, then add the Heap,
 *     then the Queue, then the Graph.
 *   • Use the display() methods from each module to verify correctness.
 *   • Add comments in main.cpp explaining WHAT each block of code does
 *     and WHY — your team will need to explain this during evaluation.
 *
 * TODO (Developer 5)
 * ------------------
 *   [ ] Add all required #include statements
 *   [ ] Declare UserHashMap, ItemHashMap, Heap, Queue, Graph
 *   [ ] STEP 1: Insert at least 5 users
 *   [ ] STEP 2: Insert at least 10 items into map AND heap
 *   [ ] STEP 3: Build the similarity graph
 *   [ ] STEP 4: Simulate at least 15 interactions (enqueue them)
 *   [ ] STEP 5: Process the queue — update scores
 *   [ ] STEP 6: Rebuild the heap with updated scores
 *   [ ] STEP 7: Display top-5 items, user levels, and item neighbours
 *   [ ] Make sure the program compiles and runs with no errors
 */

// ── YOUR CODE GOES BELOW THIS LINE ──────────────────────────
#include "models/User.h"
#include "models/Item.h"
#include "models/Interaction.h"
#include "models/InteractionManager.h"
#include "data_structures/Queue.h"

int main() {


    return 0;
}

