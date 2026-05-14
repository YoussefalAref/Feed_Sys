#include "Heap.h"
#include <stdexcept>

Heap::Heap(int capacity) : capacity(capacity), sz(0) {
    arr = new Item[capacity];
}

Heap::~Heap() {
    delete[] arr;
}

int Heap::parent(int i)     const { return (i - 1) / 2; }
int Heap::leftChild(int i)  const { return 2 * i + 1; }
int Heap::rightChild(int i) const { return 2 * i + 2; }

void Heap::swap(int i, int j) {
    Item tmp = arr[i];
    arr[i]   = arr[j];
    arr[j]   = tmp;
}

void Heap::heapifyUp(int i) {
    while (i > 0) {
        int p = parent(i);
        if (arr[i].getPopularityScore() > arr[p].getPopularityScore()) {
            swap(i, p);
            i = p;
        } else {
            break;
        }
    }
}

void Heap::heapifyDown(int i) {
    while (true) {
        int largest = i;
        int l = leftChild(i);
        int r = rightChild(i);

        if (l < sz && arr[l].getPopularityScore() > arr[largest].getPopularityScore())
            largest = l;
        if (r < sz && arr[r].getPopularityScore() > arr[largest].getPopularityScore())
            largest = r;

        if (largest == i) break;
        swap(i, largest);
        i = largest;
    }
}

void Heap::insert(const Item& item) {
    if (sz >= capacity)
        throw std::overflow_error("Heap is full");
    arr[sz] = item;
    heapifyUp(sz);
    sz++;
}

Item Heap::extractMax() {
    if (isEmpty())
        throw std::underflow_error("Heap is empty");
    Item maxItem = arr[0];
    arr[0] = arr[sz - 1];
    sz--;
    if (sz > 0) heapifyDown(0);
    return maxItem;
}

const Item& Heap::peekMax() const {
    if (isEmpty())
        throw std::underflow_error("Heap is empty");
    return arr[0];
}

bool Heap::isEmpty() const { return sz == 0; }
int  Heap::getSize() const { return sz; }
