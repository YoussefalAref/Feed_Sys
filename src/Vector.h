#ifndef VECTOR_H
#define VECTOR_H
#include <iostream>
using namespace std;


template<typename T>
class Vector{
    private:
    T* arr;
    int capacity;
    int size;
    void resize();
    public:
    Vector();
    ~Vector();
    void push_back(T value);
    void pop_back();
    T get(int index) const;
    void remove(int index);
    int getCapacity() const;
    int getSize() const;
    bool isEmpty() const;
};
#endif