#ifndef HASHMAP_H
#define HASHMAP_H

#include <iostream>
#include <string>
#include <functional>
#include <cstdint>
#include <stdexcept>
#include "Vector.h"
using namespace std;

template <typename K, typename V>
struct Node{
    K key;
    V value;
    Node* next;
    Node(K k, V v) : key(k), value(v), next(nullptr) {}
};

template<typename K>
struct HashFunc {
    size_t operator()(const K& key) const {
        return std::hash<K>()(key);
    }
};

template<typename K>
struct HashFunc<K*> {
    size_t operator()(K* key) const {
        return reinterpret_cast<size_t>(key);
    }
};

template <typename K, typename V>
class HashMap{
    private:
        Node<K, V>** table;
        int capacity;
        int size;
        double loadFactor;

        int hashFunction(K key) {
            HashFunc<K> hasher;
            return static_cast<int>(hasher(key) % static_cast<size_t>(capacity));
        }

        void rehash() {
            int oldCapacity = capacity;
            Node<K,V>** oldTable = table;
            capacity = capacity * 2;
            table = new Node<K,V>*[capacity];
            for(int i = 0; i < capacity; i++) table[i] = nullptr;
            size = 0;
            for(int i = 0; i < oldCapacity; i++){
                Node<K,V>* current = oldTable[i];
                while(current != nullptr){
                    insert(current->key, current->value);
                    current = current->next;
                }
            }
            for(int i = 0; i < oldCapacity; i++){
                Node<K,V>* current = oldTable[i];
                while(current != nullptr){
                    Node<K,V>* toDelete = current;
                    current = current->next;
                    delete toDelete;
                }
            }
            delete[] oldTable;
        }

    public:
    HashMap(int initialCapacity = 10, double lf = 0.75)
        : capacity(initialCapacity), size(0), loadFactor(lf) {
        table = new Node<K,V>*[capacity];
        for(int i = 0; i < capacity; i++) table[i] = nullptr;
    }

    ~HashMap() {
        for(int i = 0; i < capacity; i++){
            Node<K,V>* current = table[i];
            while(current != nullptr){
                Node<K,V>* toDelete = current;
                current = current->next;
                delete toDelete;
            }
        }
        delete[] table;
    }

    void insert(K key, V value) {
        int index = hashFunction(key);
        Node<K,V>* current = table[index];
        while(current != nullptr){
            if(current->key == key){
                current->value = value;
                return;
            }
            current = current->next;
        }
        Node<K,V>* newNode = new Node<K,V>(key, value);
        newNode->next = table[index];
        table[index] = newNode;
        size++;
        if((double)size / capacity >= loadFactor) rehash();
    }

    // Returns default V() when key not found
    V search(K key) {
        int index = hashFunction(key);
        Node<K,V>* current = table[index];
        while(current != nullptr){
            if(current->key == key) return current->value;
            current = current->next;
        }
        return V();
    }

    // Returns pointer to value if found, nullptr otherwise
    V* searchPointer(K key) {
        int index = hashFunction(key);
        Node<K,V>* current = table[index];
        while(current != nullptr){
            if(current->key == key) return &current->value;
            current = current->next;
        }
        return nullptr;
    }

    bool remove(K key) {
        int index = hashFunction(key);
        Node<K,V>* current = table[index];
        Node<K,V>* prev = nullptr;
        while(current != nullptr){
            if(current->key == key){
                if(prev == nullptr) table[index] = current->next;
                else prev->next = current->next;
                delete current;
                size--;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    int getSize() { return size; }
    int getCapacity() { return capacity; }

    bool contains(K key) {
        int index = hashFunction(key);
        Node<K,V>* current = table[index];
        while(current != nullptr){
            if(current->key == key) return true;
            current = current->next;
        }
        return false;
    }

    Vector<V> getAll() {
        Vector<V> allItems;
        for(int i = 0; i < capacity; i++){
            Node<K,V>* current = table[i];
            while(current != nullptr){
                allItems.push_back(current->value);
                current = current->next;
            }
        }
        return allItems;
    }

    Vector<K> getAllKeys() {
        Vector<K> allKeys;
        for(int i = 0; i < capacity; i++){
            Node<K,V>* current = table[i];
            while(current != nullptr){
                allKeys.push_back(current->key);
                current = current->next;
            }
        }
        return allKeys;
    }

    void clear() {
        for(int i = 0; i < capacity; i++){
            Node<K,V>* current = table[i];
            while(current != nullptr){
                Node<K,V>* toDelete = current;
                current = current->next;
                delete toDelete;
            }
            table[i] = nullptr;
        }
        size = 0;
    }

    void display() {}

    void forEach(void (*callback)(K, V)) {
        for(int i = 0; i < capacity; i++){
            Node<K,V>* current = table[i];
            while(current != nullptr){
                callback(current->key, current->value);
                current = current->next;
            }
        }
    }
};

#endif
