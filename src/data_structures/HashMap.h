#ifndef HASHMAP_H
#define HASHMAP_H

#include <iostream>
#include <string>
using namespace std;

//Node struct to handle collisions using chaining
template <typename K, typename V>
struct Node{
    K key;
    V value;
    Node* next;

    Node(K k, V v) : key(k), value(v), next(nullptr) {}
};

/*HashMap class: uses chaining for collision resolution and its time complexity is:
INSERT: O(1) average and O(n) worst case, 
SEARCH: O(1) average and O(n) worst case, 
REMOVE: O(1) average and O(n) worst case, 
REHASH: O(n)*/
template <typename K, typename V>
class HashMap{
    private:
        Node<K, V>** table;
        int capacity;
        int size;
        double loadFactor;
        int hashFunction(K key); // Hash function to fund the bucket index
        void rehash();
    public:
    HashMap(int initialCapacity = 10, double loadFactor = 0.75);
    ~HashMap();
    void insert(K key, V value);
    V search(K key);
    bool remove(K key);
    int getSize();
    int getCapacity();
    void display();
};
#endif




