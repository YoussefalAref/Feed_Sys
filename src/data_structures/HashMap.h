#ifndef HASH_MAP_H
#define HASH_MAP_H

/*
 * ============================================================
 *  FILE: src/data_structures/HashMap.h
 *  OWNER: Developer 1 (HashMap — Users + Items)
 * ============================================================
 *
 * PURPOSE
 * -------
 * A HashMap (also called a Hash Table) gives you O(1) average-time
 * lookup, insertion, and deletion.
 * In this project you will create TWO instances of this map:
 *   • UserHashMap   — maps  userID (int)  →  User   object
 *   • ItemHashMap   — maps  itemID (int)  →  Item   object
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: What is a Hash Map?
 *
 *   Imagine a big array of "buckets".
 *   To store a key-value pair:
 *     1. Run the key through a HASH FUNCTION → get an index.
 *     2. Store the value in the bucket at that index.
 *   To look up a key:
 *     1. Run the key through the same hash function → get the index.
 *     2. Check the bucket at that index.
 *
 *   This gives us O(1) average time — no need to scan the whole array!
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: Hash Collisions
 *
 *   Two different keys can produce the SAME index — this is a collision.
 *   There are two common strategies to handle collisions:
 *
 *   1. Separate Chaining (RECOMMENDED for this project)
 *      Each bucket holds a LINKED LIST of (key, value) pairs.
 *      If two keys hash to the same bucket, both are stored in the list.
 *
 *   2. Open Addressing
 *      If a bucket is full, probe the next bucket (linear/quadratic).
 *
 *   Use Separate Chaining because it is simpler and works well
 *   even when the table is full.
 *
 * ─────────────────────────────────────────────────────────────
 * LEARNING NOTE: Templates in C++
 *
 *   A template lets you write ONE generic class that works for
 *   different data types.  For example:
 *
 *     template <typename K, typename V>
 *     struct HashMap { ... };
 *
 *   Then you can do:
 *     HashMap<int, User>  userMap;
 *     HashMap<int, Item>  itemMap;
 *
 *   Inside the struct, use K for the key type and V for the value type.
 *
 * ─────────────────────────────────────────────────────────────
 *
 * INTERNAL STRUCTURE YOU NEED TO DESIGN
 * ----------------------------------------
 *   Step 1 — Define a Node struct (for the linked list in each bucket):
 *     Node:
 *       K key
 *       V value
 *       Node* next   ← pointer to the next node in the chain
 *
 *   Step 2 — Define the HashMap struct / class:
 *     Fields:
 *       Node** table    ← array of Node pointers (the buckets)
 *       int    capacity ← total number of buckets (start with e.g. 101)
 *       int    size     ← number of key-value pairs currently stored
 *
 * HASH FUNCTION GUIDANCE
 * ----------------------
 *   For integer keys (our use-case):
 *     index = key % capacity
 *
 *   Why use a prime number for capacity?
 *     Primes reduce clustering and spread keys more evenly.
 *     Suggested starting capacity: 101
 *
 * METHODS YOU NEED TO IMPLEMENT
 * --------------------------------
 *
 *   HashMap(int cap = 101)
 *     Constructor.
 *     Allocate the 'table' array of size 'cap'.
 *     Set every bucket to nullptr (empty).
 *
 *   ~HashMap()
 *     Destructor.
 *     Loop through every bucket.
 *     Delete all nodes in each linked list to free memory.
 *     Then delete the table array itself.
 *
 *   int hashFunction(K key)
 *     Returns:  key % capacity
 *     This gives us the bucket index for a given key.
 *
 *   void insert(K key, V value)
 *     Steps:
 *       1. Compute index = hashFunction(key)
 *       2. Check if key already exists in the chain at table[index]
 *          → If yes: UPDATE its value (don't create a duplicate)
 *       3. If no: create a new Node and prepend it to the chain
 *       4. Increment 'size'
 *       5. If load factor (size / capacity) > 0.7 → call rehash()
 *
 *   V* search(K key)
 *     Returns a POINTER to the value if found, or nullptr if not found.
 *     Steps:
 *       1. Compute index = hashFunction(key)
 *       2. Traverse the linked list at table[index]
 *       3. If a node with the matching key is found, return &(node->value)
 *       4. If not found, return nullptr
 *     NOTE: Returning a pointer (not a copy) lets the caller modify the value.
 *
 *   bool remove(K key)
 *     Steps:
 *       1. Compute index = hashFunction(key)
 *       2. Traverse the chain looking for the key
 *       3. If found: unlink the node from the list, delete it, decrement size
 *          return true
 *       4. If not found: return false
 *
 *   void rehash()
 *     Called automatically when the table is getting full.
 *     Steps:
 *       1. Double the capacity (newCap = capacity * 2)
 *       2. Allocate a new table of size newCap
 *       3. Re-insert every existing key-value pair into the new table
 *          (must re-hash each key with the new capacity)
 *       4. Delete the old table
 *       5. Update capacity
 *
 *   void display()
 *     Debugging helper: print all non-empty buckets and their chains.
 *
 *   int getSize()
 *     Returns the current number of key-value pairs stored.
 *
 * COMPLEXITY SUMMARY
 * ------------------
 *   Operation   Average   Worst (all keys collide)
 *   insert      O(1)      O(n)
 *   search      O(1)      O(n)
 *   remove      O(1)      O(n)
 *   rehash      O(n)      O(n)
 *
 * TESTING CHECKLIST (Developer 1)
 * --------------------------------
 *   [ ] Insert 100 users — print the size, verify it equals 100
 *   [ ] Search for users by ID — make sure you get the right user back
 *   [ ] Remove some users — verify they are no longer found
 *   [ ] Force a rehash (insert > 70 items into a capacity-101 table)
 *   [ ] Test what happens when you insert the same key twice
 *
 * TODO (Developer 1)
 * ------------------
 *   [ ] Write the Node struct (inside the HashMap class, as a nested struct)
 *   [ ] Write the HashMap struct/class with all fields above
 *   [ ] Implement the constructor and destructor
 *   [ ] Implement hashFunction()
 *   [ ] Implement insert()
 *   [ ] Implement search()
 *   [ ] Implement remove()
 *   [ ] Implement rehash()
 *   [ ] Implement display() for debugging
 *   [ ] Write a test block in main.cpp that exercises all operations
 *   [ ] Document your hash function choice and collision strategy
 */

// ── YOUR CODE GOES BELOW THIS LINE ──────────────────────────


#endif // HASH_MAP_H
