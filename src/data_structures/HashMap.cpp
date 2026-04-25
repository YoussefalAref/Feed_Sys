#include "HashMap.h"
#include <iostream>
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
// Hash functions
// Specialised per key type so the template works for both int and string keys.
// ---------------------------------------------------------------------------

// Polynomial rolling hash for string keys — distributes characters evenly.
// prime = 31 is a common choice for lowercase ASCII; mod keeps index in range.
// O(|key|).
template <>
int HashMap<std::string, std::string>::hashFunction(const std::string& key) const {
    int hash  = 0;
    int prime = 31;
    for (char c : key)
        hash = (hash * prime + c) % capacity_;
    return hash;
}

// Modulo hash for integer keys — O(1).
// abs() prevents negative indices from negative keys.
template <>
int HashMap<int, std::string>::hashFunction(const int& key) const {
    return (key < 0 ? -key : key) % capacity_;
}

// Generic fallback — callers using other key types must specialise this.
template <typename K, typename V>
int HashMap<K, V>::hashFunction(const K& key) const {
    return 0; // override by adding a specialisation for your key type
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
template <typename K, typename V>
HashMap<K, V>::HashMap(int initialCapacity, double loadFactor)
    : capacity_(initialCapacity), size_(0), loadFactor_(loadFactor)
{
    table_ = new Node<K, V>*[capacity_];
    for (int i = 0; i < capacity_; ++i)
        table_[i] = nullptr;
}

// ---------------------------------------------------------------------------
// Destructor — frees every node in every chain, then the bucket array.
// O(capacity + n).
// ---------------------------------------------------------------------------
template <typename K, typename V>
HashMap<K, V>::~HashMap() {
    for (int i = 0; i < capacity_; ++i) {
        Node<K, V>* cur = table_[i];
        while (cur) {
            Node<K, V>* next = cur->next;
            delete cur;
            cur = next;
        }
    }
    delete[] table_;
}

// ---------------------------------------------------------------------------
// insert
// Hash the key to find the bucket, walk its chain:
//   - Key found  → update value in place (upsert).
//   - Key absent → prepend a new node at the chain head (O(1) — no tail walk).
// After insertion check the load factor; rehash if threshold exceeded.
// O(1) average.
// ---------------------------------------------------------------------------
template <typename K, typename V>
void HashMap<K, V>::insert(const K& key, const V& value) {
    int idx = hashFunction(key);

    for (Node<K, V>* cur = table_[idx]; cur; cur = cur->next) {
        if (cur->key == key) {
            cur->value = value;
            return;
        }
    }

    Node<K, V>* node = new Node<K, V>(key, value);
    node->next  = table_[idx];
    table_[idx] = node;
    ++size_;

    if (static_cast<double>(size_) / capacity_ >= loadFactor_)
        rehash();
}

// ---------------------------------------------------------------------------
// search
// Returns the value for key, or a default-constructed V() if not found.
// Matches your original silent-default behaviour; use update() when you need
// to distinguish "absent" from a legitimately default value.
// O(1) average.
// ---------------------------------------------------------------------------
template <typename K, typename V>
V HashMap<K, V>::search(const K& key) {
    int idx = hashFunction(key);

    for (Node<K, V>* cur = table_[idx]; cur; cur = cur->next) {
        if (cur->key == key)
            return cur->value;
    }
    return V(); // key not found — return default
}

// ---------------------------------------------------------------------------
// update
// Like search, but mutates the value and throws if the key is absent.
// Use this when you need a strict "key must already exist" guarantee.
// O(1) average.
// ---------------------------------------------------------------------------
template <typename K, typename V>
void HashMap<K, V>::update(const K& key, const V& value) {
    int idx = hashFunction(key);

    for (Node<K, V>* cur = table_[idx]; cur; cur = cur->next) {
        if (cur->key == key) {
            cur->value = value;
            return;
        }
    }
    throw std::out_of_range("HashMap::update — key not found");
}

// ---------------------------------------------------------------------------
// remove
// Trailing-pointer splice: prev trails one step behind cur so we can unlink
// cur without a doubly-linked list or a special case for the head node.
// Returns true if the key was found and removed, false otherwise.
// O(1) average.
// ---------------------------------------------------------------------------
template <typename K, typename V>
bool HashMap<K, V>::remove(const K& key) {
    int          idx  = hashFunction(key);
    Node<K, V>*  prev = nullptr;
    Node<K, V>*  cur  = table_[idx];

    while (cur) {
        if (cur->key == key) {
            if (prev)
                prev->next  = cur->next;   // splice from middle or tail
            else
                table_[idx] = cur->next;   // splice head of chain

            delete cur;
            --size_;
            return true;
        }
        prev = cur;
        cur  = cur->next;
    }
    return false; // key not found
}

// ---------------------------------------------------------------------------
// rehash
// Triggered automatically by insert() when load factor is exceeded.
// Doubles capacity, allocates a fresh table, and reinserts every existing
// node so keys are redistributed across the larger bucket array.
// Old nodes are freed after reinsertion — they are NOT reused, because their
// hash index changes with the new capacity.
// O(n).
// ---------------------------------------------------------------------------
template <typename K, typename V>
void HashMap<K, V>::rehash() {
    int           oldCapacity = capacity_;
    Node<K, V>**  oldTable    = table_;

    capacity_ = capacity_ * 2;
    table_    = new Node<K, V>*[capacity_];
    for (int i = 0; i < capacity_; ++i)
        table_[i] = nullptr;
    size_ = 0; // insert() will recount

    for (int i = 0; i < oldCapacity; ++i) {
        Node<K, V>* cur = oldTable[i];
        while (cur) {
            insert(cur->key, cur->value); // reinsert into new table
            cur = cur->next;
        }
    }

    // Free old nodes (insert() allocated new ones above)
    for (int i = 0; i < oldCapacity; ++i) {
        Node<K, V>* cur = oldTable[i];
        while (cur) {
            Node<K, V>* next = cur->next;
            delete cur;
            cur = next;
        }
    }
    delete[] oldTable;
}

// ---------------------------------------------------------------------------
// display — prints every bucket and its chain. Useful for debugging.
// O(capacity + n).
// ---------------------------------------------------------------------------
template <typename K, typename V>
void HashMap<K, V>::display() const {
    for (int i = 0; i < capacity_; ++i) {
        std::cout << "Bucket " << i << ": ";
        Node<K, V>* cur = table_[i];
        while (cur) {
            std::cout << "[" << cur->key << " -> " << cur->value << "] ";
            cur = cur->next;
        }
        std::cout << "\n";
    }
}

// ---------------------------------------------------------------------------
// Explicit instantiations — add more as needed for other key/value types.
// ---------------------------------------------------------------------------
template class HashMap<std::string, std::string>;
template class HashMap<int, std::string>;


