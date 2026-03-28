#include "Heap.h"
#include <iostream>
using namespace std;

// helper functions 

// return parent and children indices

int Heap::parent(int i) const { return (i - 1) / 2; }

int Heap::leftChild(int i) const { return 2 * i + 1; }

int Heap::rightChild(int i) const { return 2 * i + 2; }

//swap items needed for heapify functions

void Heap::swap(Item& a, Item& b) {
    Item temp = a;
    a = b;
    b = temp;
}

// resize / if the capacity is full, double it and copy over the items

void Heap::resize() {
    capacity *= 2;
    Item* newData = new Item[capacity];

    for (int i = 0; i < size; i++) { newData[i] = data[i]; }

    delete[] data;
    data = newData;
}

// constructor / destructor 

Heap::Heap(int cap) {
    capacity = cap;
    size = 0;
    data = new Item[capacity];
}

Heap::~Heap() {
    delete[] data;
}

// heapify up based on item's popularity score

void Heap::heapifyUp(int index) {
    while (index > 0 &&
           data[index].getPopularityScore() >
           data[parent(index)].getPopularityScore()) {

        swap(data[index], data[parent(index)]);
        index = parent(index);
    }
}

// heapify down

void Heap::heapifyDown(int index) {
    while (true) {
        int left = leftChild(index);
        int right = rightChild(index);
        int largest = index;

        if (left < size &&
            data[left].getPopularityScore() >
            data[largest].getPopularityScore()) {
            largest = left;
        }

        if (right < size &&
            data[right].getPopularityScore() >
            data[largest].getPopularityScore()) {
            largest = right;
        }

        if (largest == index)
            break;

        swap(data[index], data[largest]);
        index = largest;
    }
}

// insert / add a new item

void Heap::insert(Item item) {
    if (size == capacity) {
        resize();
    }

    data[size] = item;
    size++;

    heapifyUp(size - 1);
}

// extract max / most popular item

Item Heap::extractMax() {
    if (isEmpty()) {
        cout << "Error: Heap is empty!" << endl;
        return Item(); // assumes default constructor exists
    }

    Item maxItem = data[0];

    data[0] = data[size - 1];
    size--;

    if (size > 0) {
        heapifyDown(0);
    }

    return maxItem;
}

// peek max / most popular item

Item Heap::peekMax() const {
    if (isEmpty()) {
        cout << "Error: Heap is empty!" << endl;
        return Item();
    }

    return data[0];
}

// utility functions

bool Heap::isEmpty() const {
    return size == 0;
}

int Heap::getSize() const {
    return size;
}

void Heap::display() const {
    cout << "Heap Contents:\n";
    for (int i = 0; i < size; i++) {
        cout << data[i].getPopularityScore() << " ";
    }
    cout << endl;
}