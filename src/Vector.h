#ifndef VECTOR_H
#define VECTOR_H
#include <iostream>
#include <iterator>
#include <stdexcept>
using namespace std;

template<typename T>
class Vector{
    private:
    T* arr;
    int capacity;
    int sz;

    void resize() {
        capacity *= 2;
        T* newArr = new T[capacity];
        for(int i = 0; i < sz; i++) newArr[i] = arr[i];
        delete[] arr;
        arr = newArr;
    }

    public:
    Vector() : capacity(10), sz(0) {
        arr = new T[capacity];
    }

    Vector(const Vector& other) : capacity(other.capacity), sz(other.sz) {
        arr = new T[capacity];
        for(int i = 0; i < sz; i++) arr[i] = other.arr[i];
    }

    Vector& operator=(const Vector& other) {
        if(this != &other) {
            delete[] arr;
            capacity = other.capacity;
            sz = other.sz;
            arr = new T[capacity];
            for(int i = 0; i < sz; i++) arr[i] = other.arr[i];
        }
        return *this;
    }

    ~Vector() { delete[] arr; }

    void push_back(T value) {
        if(sz == capacity) resize();
        arr[sz] = value;
        sz++;
    }

    void pop_back() {
        if(sz == 0) throw std::out_of_range("Vector is empty");
        sz--;
    }

    T get(int index) const {
        if(index >= 0 && index < sz) return arr[index];
        throw std::out_of_range("Index out of bounds");
    }

    void remove(int index) {
        if(index < 0 || index >= sz) throw std::out_of_range("Index out of bounds");
        for(int j = index; j < sz - 1; j++) arr[j] = arr[j+1];
        sz--;
    }

    int getCapacity() const { return capacity; }
    int getSize() const { return sz; }
    bool isEmpty() const { return sz == 0; }
    bool empty() const { return sz == 0; }

    void set(int index, T value) {
        if(index >= 0 && index < sz) arr[index] = value;
    }

    void reserve(int newCapacity) {
        if(newCapacity > capacity) {
            capacity = newCapacity;
            T* newArr = new T[capacity];
            for(int i = 0; i < sz; i++) newArr[i] = arr[i];
            delete[] arr;
            arr = newArr;
        }
    }

    void clear() { sz = 0; }

    T& operator[](int index) { return arr[index]; }
    const T& operator[](int index) const { return arr[index]; }

    // Raw pointer access
    T* data() { return arr; }
    const T* data() const { return arr; }

    // Forward iterators
    T* begin() { return arr; }
    T* end() { return arr + sz; }
    const T* begin() const { return arr; }
    const T* end() const { return arr + sz; }

    // Reverse iterators
    using reverse_iterator = std::reverse_iterator<T*>;
    using const_reverse_iterator = std::reverse_iterator<const T*>;

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend()   { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend()   const { return const_reverse_iterator(begin()); }
};

#endif
