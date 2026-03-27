#ifndef HEAP_H
#define HEAP_H

#include "../models/Item.h"

class Heap {
private:
    Item* data;
    int size;
    int capacity;

    // helper functions to calculate indices
    int parent(int i) const ;
    int leftChild(int i) const ;
    int rightChild(int i) const ;

    // main internal functions
    void resize();
    void swap(Item& a, Item& b);
    void heapifyUp(int index);
    void heapifyDown(int index);

public:

    Heap(int cap = 100);
    ~Heap();

    // main heap functions
    void insert(Item item);
    Item extractMax();
    Item peekMax() const;

    // utility functions
    bool isEmpty() const;
    int getSize() const;
    void display() const;
};

#endif 