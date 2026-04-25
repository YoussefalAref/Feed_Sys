#include "HashMap.h"
#include "Item.h"
#include <iostream>
#include <string>
using namespace std;

//Return the bucket number for a string input
template<>
int HashMap<string, Item>::hashFunction(string key){
    int hash = 0;
    int prime = 77;
    for(int i = 0; i<key.length(); i++){
        hash = (hash * prime + key[i]) % capacity;
    }
    return hash;
}

//Constructor to initialize the table with empty buckets
template<typename K, typename V>
HashMap<K,V>::HashMap(int initialCapacity, double loadFactor){
    this->capacity = initialCapacity;
    this->loadFactor = loadFactor;
    this->size = 0;
    table = new Node<K,V>*[capacity];
    for(int i = 0; i < capacity; i++){
        table[i] = nullptr;
    }
}

//Destructor to free all nodes and then delete the table
template<typename K, typename V>
HashMap<K,V>:: ~HashMap(){
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

// insert a key and value pair into the hash map, if key already exists, update the value, if key is new, add a new node to the corresponding bucket
template<typename K, typename V>
void HashMap<K,V>::insert(K key, V value){
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
    if((double)size / capacity >= loadFactor){
        rehash();
    }
}

// search for a key in the hash map and return its value, if key is not found, return default value of V
template<typename K, typename V>
V HashMap<K,V>::search(K key){
    int index = hashFunction(key);
    Node<K,V>* current = table[index];

    while(current != nullptr){
        if(current->key == key){
            return current->value;
        }
        current = current->next;
    }
    return V();
}

// remove a key from the hash map, return true if the key was found and removed, false otherwise
template<typename K, typename V>
bool HashMap<K,V>::remove(K key){
    int index = hashFunction(key);
    Node<K,V>* current = table[index];
    Node<K,V>* prev = nullptr;

    while(current != nullptr){
        if(current->key == key){
            if(prev == nullptr){
                table[index] = current->next;
            } else{
                prev->next = current->next;
            }
            delete current;
            size--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

// rehash the table by creating a new table with double the capacity and reinserting all existing key-value pairs into the new table
template<typename K, typename V>
void HashMap<K, V>:: rehash(){
    int oldCapacity = capacity;
    Node<K,V>** oldTable = table;

    capacity = capacity * 2;
    table = new Node<K,V>*[capacity];

    for(int i = 0; i < capacity; i++){
        table[i] = nullptr;
    }
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

// this prints the entire table bucket by bucker.
template<typename K, typename V>
void HashMap<K, V>:: display(){
    for(int i = 0; i < capacity; i++){
        cout<<"Bucket "<<i<<": ";
        Node<K,V>* current = table[i];
        while(current != nullptr){
            cout<<"["<<current->key<<" -> "<<current->value<<"] ";
            current = current->next;
        }
        cout<<endl;
    }
}

//Getters
template <typename K, typename V>
int HashMap<K, V>::getSize(){
    return size;
}

template <typename K, typename V>
int HashMap<K, V>::getCapacity(){
    return capacity;
}

template<typename K, typename V>
Vector<V> HashMap<K,V>::getAll(){
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
// explicit instantiation of the template class
template class HashMap<string, Item>;






