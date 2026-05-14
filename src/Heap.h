#ifndef HEAP_H
#define HEAP_H

#include "Item.h"

/*
 * Max-heap of Item objects, ordered by popularityScore (highest = root).
 * Used by the trending algorithm to extract the top-k products in
 * O(n + k log n) time.
 */
class Heap {
private:
    Item* arr;
    int   capacity;
    int   sz;

    int parent(int i)     const;
    int leftChild(int i)  const;
    int rightChild(int i) const;

    void swap(int i, int j);
    void heapifyUp(int i);
    void heapifyDown(int i);

public:
    explicit Heap(int capacity);
    ~Heap();

    void insert(const Item& item);
    Item extractMax();
    const Item& peekMax() const;

    bool isEmpty() const;
    int  getSize() const;
};

#endif
