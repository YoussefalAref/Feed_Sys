#ifndef QUEUE_H
#define QUEUE_H

#include "../models/Interaction.h"
class Queue {
private:
    struct Node {
        Interaction data;
        Node* next;
        Node(Interaction interaction) : data(interaction), next(nullptr) {}
    };

    Node* front;
    Node* back;
    int size;
public:
    Queue();
    ~Queue();
    void enqueue(Interaction interaction);
    Interaction dequeue();
    Interaction peek();
    bool isEmpty();
    int getSize();
    void display();
};
/*
 * ============================================================
 *  FILE: src/data_structures/Queue.h
 *  OWNER: Developer 3 (Queue — Activity Tracking)
 * ============================================================
 *
 * PURPOSE
 * -------
 * A Queue records every user interaction in the order it happened.
 * It follows the FIFO rule: First In, First Out.
 * The oldest interaction is always at the front, the newest at the back.
 *
 * In this system the Queue:
 *   1. Stores Interaction objects as they occur (enqueue)
 *   2. Processes them in order to update popularity and activity scores (dequeue)
 *   3. Provides timestamps so Milestone 2 can apply time decay
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: Queue vs Stack
 *
 *   Queue → FIFO (First In, First Out)
 *     Think of a supermarket checkout line:
 *     the first person who joins is the first to leave.
 *
 *   Stack → LIFO (Last In, First Out)
 *     Think of a pile of plates:
 *     the last plate placed on top is the first one taken.
 *
 *   We use a Queue because we want to process interactions
 *   in chronological order — oldest first.
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: Linked List vs Array for a Queue
 *
 *   Array-based queue:
 *     Simple but wastes space as elements are dequeued from the front.
 *     (Can be solved with a circular buffer — advanced.)
 *
 *   Linked list-based queue (RECOMMENDED for this project):
 *     Each node holds one Interaction and a pointer to the next node.
 *     'front' pointer → oldest element (dequeue from here)
 *     'back' pointer  → newest element (enqueue here)
 *     Both operations are O(1) — no shifting needed.
 *
 * ─────────────────────────────────────────────────────────────
 *
 * WHAT TO STORE IN THE QUEUE
 * ---------------------------
 *   The queue stores Interaction objects (from Interaction.h).
 *   You will need to #include "../models/Interaction.h"
 *
 * INTERNAL STRUCTURE
 * ------------------
 *   Step 1 — Define a Node struct:
 *     Node:
 *       Interaction data   ← the interaction stored in this node
 *       Node*       next   ← pointer to the next (newer) node
 *
 *   Step 2 — Define the Queue struct/class:
 *     Fields:
 *       Node* front   ← points to the OLDEST interaction (dequeue end)
 *       Node* back    ← points to the NEWEST interaction (enqueue end)
 *       int   size    ← number of interactions currently in the queue
 *
 * METHODS YOU NEED TO IMPLEMENT
 * --------------------------------
 *
 *   Queue()
 *     Constructor.
 *     Set front = nullptr, back = nullptr, size = 0.
 *
 *   ~Queue()
 *     Destructor.
 *     Loop through all nodes from front to back and delete each one.
 *     This prevents memory leaks.
 *
 *   void enqueue(Interaction interaction)
 *     Add a new interaction to the BACK of the queue.
 *     Steps:
 *       1. Create a new Node with the given interaction.
 *       2. If the queue is empty (back == nullptr), set front = back = newNode.
 *       3. Otherwise: set back->next = newNode, then back = newNode.
 *       4. Increment size.
 *
 *   Interaction dequeue()
 *     Remove and return the interaction at the FRONT of the queue.
 *     Steps:
 *       1. If queue is empty, print an error (or throw an exception).
 *       2. Save front->data.
 *       3. Move front to front->next.
 *       4. Delete the old front node.
 *       5. If front is now nullptr, set back = nullptr too (queue is empty).
 *       6. Decrement size.
 *       7. Return the saved interaction.
 *
 *   Interaction peek()
 *     Returns front->data WITHOUT removing it.
 *     Use this to look at the oldest interaction without processing it yet.
 *
 *   bool isEmpty()
 *     Returns true if size == 0  (or front == nullptr).
 *
 *   int getSize()
 *     Returns the current number of interactions in the queue.
 *
 *   void display()
 *     Print all interactions from front to back.
 *     Useful for debugging — shows the order in which they will be processed.
 *
 * WHY O(1) FOR ENQUEUE AND DEQUEUE?
 * -----------------------------------
 *   Because we maintain direct pointers to both ends (front and back),
 *   we never need to traverse the list to add or remove an element.
 *   Each operation is a constant number of pointer updates → O(1).
 *
 * TIME DECAY (Milestone 2 Preview)
 * ----------------------------------
 *   Each Interaction stores a timestamp (time_t).
 *   In Milestone 2, you will compute:
 *     decayedScore = originalScore * e^(-lambda * ageInSeconds)
 *   For now, just make sure the timestamp is stored correctly.
 *
 * TESTING CHECKLIST (Developer 3)
 * --------------------------------
 *   [ ] Enqueue 20 different Interaction objects
 *   [ ] Print the queue (display()) — verify order is correct
 *   [ ] Dequeue 10 — verify the OLDEST are removed first
 *   [ ] Test peek() — make sure it doesn't remove the element
 *   [ ] Test isEmpty() before and after all elements are dequeued
 *   [ ] Test what happens when you dequeue from an empty queue
 *
 * TODO (Developer 3)
 * ------------------
 *   [ ] Write the Node struct (key + next pointer)
 *   [ ] Write the Queue struct/class with front, back, size
 *   [ ] Implement constructor and destructor (no memory leaks!)
 *   [ ] Implement enqueue()
 *   [ ] Implement dequeue()
 *   [ ] Implement peek(), isEmpty(), getSize(), display()
 *   [ ] Test all operations in main.cpp
 *   [ ] Document: explain FIFO and why a linked list is used
 */

// ── YOUR CODE GOES BELOW THIS LINE ──────────────────────────


#endif // QUEUE_H
