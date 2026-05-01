#ifndef HashMap_H
#define HashMap_H

#include <iostream>
#include <string>
#include <stdexcept>

// Node in the chained linked list stored at each bucket.
// Chaining lets multiple keys that hash to the same bucket coexist without
// evicting each other — each bucket is just the head of a short linked list.
template <typename K, typename V>
struct Node {
    K     key;
    V     value;
    Node* next;

    Node(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
};

/*
 * HashMap<K, V> — separate-chaining hash table with dynamic rehashing.
 *
 * Collision handling : every bucket holds a singly-linked list (chain).
 *   Keys that hash to the same bucket are appended to that chain.
 *
 * Rehashing : when (size / capacity) exceeds loadFactor the table is rebuilt
 *   at 2× capacity so chains stay short and average-case complexity holds.
 *
 * Complexity (average / worst):
 *   insert  — O(1) / O(n)
 *   search  — O(1) / O(n)
 *   update  — O(1) / O(n)
 *   remove  — O(1) / O(n)
 *   rehash  — O(n)  (amortised O(1) per insert)
 */
template <typename K, typename V>
class HashMap {
public:
    explicit HashMap(int initialCapacity = 10, double loadFactor = 0.75);
    ~HashMap();

    HashMap(const HashMap&)            = delete;
    HashMap& operator=(const HashMap&) = delete;

    void insert(const K& key, const V& value);  // upsert; triggers rehash if needed
    V    search(const K& key);                  // returns V() if key absent
    void update(const K& key, const V& value);  // throws std::out_of_range if absent
    bool remove(const K& key);                  // true = found & removed

    void display()    const;
    int  getSize()    const { return size_; }
    int  getCapacity()const { return capacity_; }

    // Returns the head node of bucket[index] for external iteration.
    // Used by modules (e.g. Authentication) that need to scan all values
    // for non-key fields such as email. Read-only — do not modify nodes.
    const Node<K, V>* getBucket(int index) const { return table_[index]; }

private:
    Node<K, V>** table_;
    int          capacity_;
    int          size_;
    double       loadFactor_;

    int  hashFunction(const K& key) const;
    void rehash();
};

#include "HashMap.cpp"  // template definitions must be visible at instantiation
#endif


