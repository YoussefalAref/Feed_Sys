#ifndef HEAP_H
#define HEAP_H

#include "../models/Item.h"

class Heap {
private:
    Item* data;
    int size;
    int capacity;

    // helper functions to calculate indices
    int parent(int i);
    int leftChild(int i);
    int rightChild(int i);

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
    Item peekMax();

    // utility functions
    bool isEmpty();
    int getSize();
    void display();
};

#endif 