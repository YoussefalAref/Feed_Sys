#ifndef STL_HELPER_H
#define STL_HELPER_H

#include "Vector.h"
#include "HashMap.h"
#include <string>
#include <ctime>

// Custom pair-like class for storing score pairs
template<typename T>
struct Pair {
    T first;
    double second;

    Pair() : first(), second(0.0) {}
    Pair(T f, double s) : first(f), second(s) {}
};

// Bubble sort with a template comparator (supports lambdas and function pointers)
template<typename T, typename Comp>
void vectorSort(Vector<T>& vec, Comp compare) {
    int n = vec.getSize();
    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (compare(vec[j+1], vec[j])) {
                T temp = vec[j];
                vec[j] = vec[j+1];
                vec[j+1] = temp;
            }
        }
    }
}

// Find element in vector, returns index or -1 if not found
template<typename T>
int vectorFind(const Vector<T>& vec, const T& value) {
    for (int i = 0; i < vec.getSize(); ++i) {
        if (vec[i] == value) return i;
    }
    return -1;
}

// String to integer conversion
inline int stringToInt(const std::string& str) {
    try {
        size_t idx;
        return std::stoi(str, &idx);
    } catch (...) {
        return 0;
    }
}

// Integer to string conversion
inline std::string intToString(int value) {
    return std::to_string(value);
}

// Character to lowercase
inline char toLowerChar(char c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

// String to lowercase
inline std::string stringToLower(const std::string& str) {
    std::string result = str;
    for (int i = 0; i < (int)result.length(); ++i)
        result[i] = toLowerChar(result[i]);
    return result;
}

// Maximum of two numbers
template<typename T>
inline T maxVal(T a, T b) { return a > b ? a : b; }

// Minimum of two numbers
template<typename T>
inline T minVal(T a, T b) { return a < b ? a : b; }

// Structure for iteration over HashMap entries
template<typename K, typename V>
struct MapEntry {
    K key;
    V value;
};

#endif
