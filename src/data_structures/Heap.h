#ifndef HEAP_H
#define HEAP_H

/*
 * ============================================================
 *  FILE: src/data_structures/Heap.h
 *  OWNER: Developer 2 (Heap — Ranking System)
 * ============================================================
 *
 * PURPOSE
 * -------
 * A MaxHeap keeps the highest-priority element always at the top.
 * In this system, "priority" = an item's popularityScore.
 *
 * Use cases:
 *   • Global trending feed  → extract top-K popular items for new users
 *   • Personalized feed     → build a temporary heap per request (Milestone 2)
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: What is a Heap?
 *
 *   A heap is a COMPLETE BINARY TREE stored as a plain array.
 *   "Complete" means every level is fully filled except possibly
 *   the last level, which is filled left-to-right.
 *
 *   MaxHeap property: Every parent node is GREATER THAN OR EQUAL
 *   to its children.  This means the maximum element is always at
 *   index 0 (the root).
 *
 *   Array representation — for a node at index i:
 *     Parent      → (i - 1) / 2
 *     Left child  → 2 * i + 1
 *     Right child → 2 * i + 2
 *
 *   Example (array: [90, 80, 70, 60, 50]):
 *         90
 *        /  \
 *       80   70
 *      / \
 *     60  50
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: heapifyUp vs heapifyDown
 *
 *   heapifyUp   — used AFTER inserting a new element.
 *     Start at the last position. While the element is GREATER than
 *     its parent, swap them and move up.  Stops at the root or when
 *     the heap property is satisfied.  O(log n).
 *
 *   heapifyDown — used AFTER extracting the maximum (root).
 *     Replace root with the last element, then push it DOWN by
 *     swapping with the LARGER child until heap property is restored.
 *     O(log n).
 *
 * ─────────────────────────────────────────────────────────────
 *
 * WHAT TO STORE IN THE HEAP
 * -------------------------
 *   The heap stores Item objects (from Item.h).
 *   Comparison is done on Item.popularityScore.
 *
 *   You will need to #include "../models/Item.h"
 *
 * INTERNAL STRUCTURE
 * ------------------
 *   Field:
 *     Item* data    ← dynamic array holding Item objects
 *     int   size    ← current number of elements
 *     int   capacity ← maximum capacity of the array
 *
 *   Start with a reasonable initial capacity (e.g. 100).
 *   Resize the array if it becomes full (double the capacity).
 *
 * METHODS YOU NEED TO IMPLEMENT
 * --------------------------------
 *
 *   Heap(int cap = 100)
 *     Constructor.
 *     Allocate 'data' array of size 'cap'.
 *     Set size = 0, capacity = cap.
 *
 *   ~Heap()
 *     Destructor.
 *     Delete the 'data' array to free memory.
 *
 *   void insert(Item item)
 *     Steps:
 *       1. If size == capacity, resize (double the array).
 *       2. Place the new item at data[size], increment size.
 *       3. Call heapifyUp(size - 1) to restore heap property.
 *
 *   Item extractMax()
 *     Returns the item with the highest popularityScore and removes it.
 *     Steps:
 *       1. Save data[0] (the maximum).
 *       2. Move data[size - 1] to data[0].
 *       3. Decrement size.
 *       4. Call heapifyDown(0) to restore heap property.
 *       5. Return the saved maximum.
 *     IMPORTANT: If the heap is empty, handle that gracefully
 *                (print an error or throw an exception).
 *
 *   Item peekMax()
 *     Returns data[0] WITHOUT removing it.
 *     Use this when you only want to CHECK the top item.
 *
 *   void heapifyUp(int index)
 *     PRIVATE helper (called after insert).
 *     While index > 0 AND data[index].popularityScore
 *           > data[parent(index)].popularityScore:
 *       swap(data[index], data[parent(index)])
 *       index = parent(index)
 *
 *   void heapifyDown(int index)
 *     PRIVATE helper (called after extractMax).
 *     Repeatedly swap with the LARGER child until no swap needed.
 *     Stop when both children are smaller or index is a leaf.
 *
 *   void display()
 *     Print all items in the heap array (not necessarily sorted).
 *     Useful for debugging.
 *
 *   bool isEmpty()
 *     Returns true if size == 0.
 *
 *   int getSize()
 *     Returns size.
 *
 * HELPER (parent / child index formulas)
 * ----------------------------------------
 *   int parent(int i)     { return (i - 1) / 2; }
 *   int leftChild(int i)  { return 2 * i + 1;   }
 *   int rightChild(int i) { return 2 * i + 2;   }
 *
 * EXTENDING FOR PERSONALISED RECOMMENDATIONS (Milestone 2 preview)
 * -----------------------------------------------------------------
 *   In Milestone 2, the scoring function will be:
 *     score = popularityScore + similarityScore + userInterestBoost
 *   You will build a TEMPORARY heap per request using this combined score.
 *   For now, just rank by popularityScore.
 *
 * COMPLEXITY SUMMARY
 * ------------------
 *   insert      O(log n)
 *   extractMax  O(log n)
 *   peekMax     O(1)
 *   build heap  O(n)   (if inserting n items one by one: O(n log n))
 *
 * TESTING CHECKLIST (Developer 2)
 * --------------------------------
 *   [ ] Insert 50 items with different popularityScores
 *   [ ] Call extractMax() 10 times — verify they come out in descending order
 *   [ ] Verify peekMax() does NOT remove the element
 *   [ ] Test with an empty heap (extractMax should not crash)
 *   [ ] Force a resize (insert more items than initial capacity)
 *
 * TODO (Developer 2)
 * ------------------
 *   [ ] Declare the Heap struct/class with the fields listed above
 *   [ ] Implement constructor and destructor
 *   [ ] Implement insert() and heapifyUp()
 *   [ ] Implement extractMax() and heapifyDown()
 *   [ ] Implement peekMax(), isEmpty(), getSize(), display()
 *   [ ] Test all operations in main.cpp
 *   [ ] Document: explain the array representation and O(log n) complexity
 */

// ── YOUR CODE GOES BELOW THIS LINE ──────────────────────────


#endif // HEAP_H
