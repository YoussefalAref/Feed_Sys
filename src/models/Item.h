#ifndef ITEM_H
#define ITEM_H

/*
 * ============================================================
 *  FILE: src/models/Item.h
 *  OWNER: Developer 5 (Integration + Models)
 * ============================================================
 *
 * PURPOSE
 * -------
 * This file defines the Item data model.
 * An Item represents any product listed on the platform
 * (e.g., laptop, phone, headphones).
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: Header Guards
 *   The lines  #ifndef ITEM_H / #define ITEM_H / #endif
 *   are called "header guards".  They prevent the compiler
 *   from including this file more than once during a build.
 *   ALWAYS add them to every .h file.
 * ─────────────────────────────────────────────────────────────
 *
 * FIELDS YOU NEED TO ADD
 * ----------------------
 *
 *   itemID          — unique identifier for the product
 *                     Suggested type: int
 *
 *   name            — product name, e.g. "Wireless Mouse"
 *                     Suggested type: string
 *
 *   category        — product category, e.g. "Electronics"
 *                     Suggested type: string
 *
 *   price           — price in local currency
 *                     Suggested type: double
 *
 *   popularityScore — the main value used by the Heap to rank items
 *                     Increases when users view / click / buy this item.
 *                     Suggested type: double  (start at 0.0)
 *
 *   stock           — how many units are available
 *                     Suggested type: int
 *
 * POPULARITY SCORE UPDATE (for reference)
 * ----------------------------------------
 *   When a user interacts with an item, increase popularityScore:
 *     view      → +1
 *     click     → +2
 *     add-to-cart → +5
 *     purchase  → +10
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: Why do we need a popularity score?
 *   The MaxHeap (see Heap.h) orders items by this score.
 *   A higher score means the item is trending / popular.
 *   New users see the top-K items from the global heap.
 * ─────────────────────────────────────────────────────────────
 *
 * METHODS TO CONSIDER (optional for Milestone 1)
 * -----------------------------------------------
 *   updateScore(int interactionType) — adds the right weight to popularityScore
 *   display()                        — prints item info to console
 *
 * HOW THIS MODEL IS USED IN THE SYSTEM
 * -------------------------------------
 *   ItemHashMap  stores  itemID  →  Item  pairs.
 *   The Heap     ranks   Item objects by popularityScore.
 *   The Graph    connects itemID nodes (similar products).
 *   The Queue    stores  Interaction objects that reference itemID.
 *
 * TODO (Developer 5)
 * ------------------
 *   [ ] Declare all fields listed above inside the struct
 *   [ ] Write a constructor that sets popularityScore = 0.0
 *   [ ] (Optional) Add updateScore() and display() methods
 */

// ── YOUR CODE GOES BELOW THIS LINE ──────────────────────────


#endif // ITEM_H
