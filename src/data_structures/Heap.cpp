#include "Heap.h"
#include <iostream>
using namespace std;

// helper functions 

int Heap::parent(int i) { return (i - 1) / 2; }

int Heap::leftChild(int i) { return 2 * i + 1; }

int Heap::rightChild(int i) { return 2 * i + 2; }

void Heap::swap(Item& a, Item& b) {
    Item temp = a;
    a = b;
    b = temp;
}

// resize 

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
           data[index].popularityScore >
           data[parent(index)].popularityScore) {

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
            data[left].popularityScore >
            data[largest].popularityScore) {
            largest = left;
        }

        if (right < size &&
            data[right].popularityScore >
            data[largest].popularityScore) {
            largest = right;
        }

        if (largest == index)
            break;

        swap(data[index], data[largest]);
        index = largest;
    }
}

// insert

void Heap::insert(Item item) {
    if (size == capacity) {
        resize();
    }

    data[size] = item;
    size++;

    heapifyUp(size - 1);
}

// extract max

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

// peek max

Item Heap::peekMax() {
    if (isEmpty()) {
        cout << "Error: Heap is empty!" << endl;
        return Item();
    }

    return data[0];
}

// utility functions

bool Heap::isEmpty() {
    return size == 0;
}

int Heap::getSize() {
    return size;
}

void Heap::display() {
    cout << "Heap Contents:\n";
    for (int i = 0; i < size; i++) {
        cout << data[i].popularityScore << " ";
    }
    cout << endl;
}