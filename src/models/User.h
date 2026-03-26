#ifndef USER_H
#define USER_H

/*
 * ============================================================
 *  FILE: src/models/User.h
 *  OWNER: Developer 5 (Integration + Models)
 * ============================================================
 *
 * PURPOSE
 * -------
 * This file defines the User data model.
 * A "model" is simply a structured bundle of data about one entity
 * in your system — in this case, a user of the platform.
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: What is a struct vs a class in C++?
 *   struct  → all members are PUBLIC by default   (use for plain data)
 *   class   → all members are PRIVATE by default  (use for behaviour)
 * For simple data containers like models, 'struct' is fine.
 * ─────────────────────────────────────────────────────────────
 *
 * FIELDS YOU NEED TO ADD
 * ----------------------
 * Think about what information you need to store for each user:
 *
 *   userID      — a unique number identifying this user
 *                 Suggested type: int
 *
 *   username    — the display name / login name
 *                 Suggested type: string  (include <string>)
 *
 *   email       — the user's email address
 *                 Suggested type: string
 *
 *   password    — stored as a hash in real systems; for now plain text
 *                 Suggested type: string
 *
 *   activityScore — computed from interactions (see formula below)
 *                 Suggested type: int or double
 *
 *   userLevel   — "Normal", "Active", or "VIP"  (derived from score)
 *                 Suggested type: string
 *
 * ACTIVITY SCORE FORMULA
 * ----------------------
 *   activityScore = (views × 1) + (clicks × 2) + (cartAdds × 5) + (purchases × 10)
 *
 *   Level thresholds:
 *     activityScore > 1000 → "VIP"
 *     activityScore > 500  → "Active"
 *     else                 → "Normal"
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: Constructor
 *   A constructor is a special function that runs automatically
 *   when you create an object.  It lets you set initial values.
 *
 *   Example:
 *     User(int id, string name, string mail, string pass)
 *     {
 *         userID   = id;
 *         username = name;
 *         ...
 *         activityScore = 0;
 *         userLevel     = "Normal";
 *     }
 * ─────────────────────────────────────────────────────────────
 *
 * METHODS TO CONSIDER (optional for Milestone 1)
 * -----------------------------------------------
 *   updateLevel()  — recalculates userLevel from activityScore
 *   display()      — prints user info to the console (useful for testing)
 *
 * HOW THIS MODEL IS USED IN THE SYSTEM
 * -------------------------------------
 *   UserHashMap  stores  userID  →  User  pairs.
 *   The Queue    stores  Interaction  objects that reference a User's ID.
 *   The Heap     uses    Item objects, not User objects directly.
 *
 * TODO (Developer 5)
 * ------------------
 *   [ ] Declare all fields listed above inside the struct
 *   [ ] Write a constructor that initialises activityScore = 0 and userLevel = "Normal"
 *   [ ] (Optional) Add an updateLevel() method
 *   [ ] (Optional) Add a display() method for debugging
 */

// ── YOUR CODE GOES BELOW THIS LINE ──────────────────────────


#endif // USER_H
