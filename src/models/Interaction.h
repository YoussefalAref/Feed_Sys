#ifndef INTERACTION_H
#define INTERACTION_H

#include <ctime>
#include <string>
enum InteractionType { VIEW, CLICK, ADD_TO_CART, PURCHASE };
class Interaction {
private:
    int userID;
    int itemID;
    InteractionType type;
    long timestamp;
public:
    //saves the new live event
    Interaction(int userID, int itemID, InteractionType type);
    // used in loading from file
    Interaction(int userID, int itemID, InteractionType type, long timestamp);
    int getUserID() const;
    int getItemID() const;
    InteractionType getType() const;
    long getTimestamp() const;
    void display() const;
};


/*
 * ============================================================
 *  FILE: src/models/Interaction.h
 *  OWNER: Developer 5 (Integration + Models)
 * ============================================================
 *
 * PURPOSE
 * -------
 * An Interaction records a single event: a user did something
 * with an item (viewed it, clicked it, added it to cart, or bought it).
 * These events are stored in the Queue (see Queue.h) and are used to:
 *   1. Update an item's popularity score
 *   2. Track a user's activity score
 *   3. Feed the recommendation engine (Milestone 2)
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: Enums
 *   Instead of using magic numbers (0 = view, 1 = click …),
 *   use an enum to give names to the interaction types:
 *
 *     enum InteractionType { VIEW, CLICK, ADD_TO_CART, PURCHASE };
 *
 *   This makes the code much easier to read and avoids mistakes.
 *   Declare the enum BEFORE the struct, inside this file.
 * ─────────────────────────────────────────────────────────────
 *
 * FIELDS YOU NEED TO ADD
 * ----------------------
 *
 *   userID          — the ID of the user who acted
 *                     Suggested type: int
 *
 *   itemID          — the ID of the item that was acted on
 *                     Suggested type: int
 *
 *   type            — what kind of interaction occurred
 *                     Suggested type: InteractionType  (enum above)
 *
 *   timestamp       — when the interaction happened
 *                     Suggested type: long  (use time(nullptr) from <ctime>)
 *                     Why long?  time() returns the number of seconds since
 *                     Jan 1 1970 (called "Unix epoch time").
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: Timestamps and Time Decay
 *   Storing a timestamp allows the system to give LESS weight to
 *   old interactions.  For example, something a user bought 6 months
 *   ago should influence recommendations less than something they
 *   bought yesterday.  This is called "time decay" and will be
 *   implemented in Milestone 2.
 *
 *   For now, just store the timestamp.
 * ─────────────────────────────────────────────────────────────
 *
 * SCORE WEIGHTS (for reference — used in main.cpp)
 * -------------------------------------------------
 *   VIEW        → +1  point to item popularity, +1  to user activity
 *   CLICK       → +2  points to item popularity, +2  to user activity
 *   ADD_TO_CART → +5  points to item popularity, +5  to user activity
 *   PURCHASE    → +10 points to item popularity, +10 to user activity
 *
 * HOW THIS MODEL IS USED IN THE SYSTEM
 * -------------------------------------
 *   The Queue (Queue.h) stores Interaction objects.
 *   main.cpp calls enqueue() each time a simulated event occurs,
 *   then dequeues interactions to update scores.
 *
 * TODO (Developer 5)
 * ------------------
 *   [ ] Declare the InteractionType enum (VIEW, CLICK, ADD_TO_CART, PURCHASE)
 *   [ ] Declare the Interaction struct with the four fields above
 *   [ ] Write a constructor that accepts (userID, itemID, type)
 *       and automatically sets timestamp = time(nullptr)
 *   [ ] (Optional) Add a display() method that prints the interaction
 */

// ── YOUR CODE GOES BELOW THIS LINE ──────────────────────────


#endif // INTERACTION_H
