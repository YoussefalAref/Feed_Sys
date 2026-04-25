#include "Vector.h"
#include "Item.h"
#include <iostream>
#include <stdexcept>
using namespace std;

template<typename T>
Vector<T>::Vector(){
    capacity = 10;
    arr = new T[capacity];
    size = 0;
}

template<typename T>
Vector<T>::~Vector(){
    delete[] arr;
}

template<typename T>
void Vector<T>::push_back(T value){
    if(size == capacity){
        resize();
    }
    arr[size] = value;
    size++;
}

template<typename T>
void Vector<T>::resize(){
    capacity *= 2;
    T* newArr = new T[capacity];
    for(int i = 0; i<size; i++){
        newArr[i] = arr[i];
    }
    delete[] arr;
    arr = newArr;
}

template<typename T>
void Vector<T>::pop_back(){
    if(size == 0){
        throw out_of_range("Vector is empty");
    }
    size--;
}

template<typename T>
T Vector<T>::get(int index) const{
    if(index >= 0 && index < size){
        return arr[index];
    }
    throw out_of_range("Index out of bounds");
}

template<typename T>
void Vector<T>::remove(int index){
    for(int i = 0; i<size; i++){
        if(i == index){
            for(int j = i; j<size-1; j++){
                arr[j] = arr[j+1];
            }
            size--;
            return;
        }
    }
    throw out_of_range("Index out of bounds");
}

template<typename T>
int Vector<T>::getCapacity() const{
    return capacity;
}

template<typename T>
int Vector<T>::getSize() const{
    return size;
}

template<typename T>
bool Vector<T>::isEmpty() const{
    if(size == 0){
        return true;
    }
    return false;
}

template class Vector<Item>;