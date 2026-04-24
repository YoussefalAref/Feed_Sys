#ifndef QUEUE_H
#define QUEUE_H

#include "../models/Interaction.h"
class Queue {
private:
    struct Node {
        Interaction data;
        Node* next;
        Node(Interaction interaction) : data(interaction), next(nullptr) {}
    };

    Node* front;
    Node* back;
    int size;
public:
    Queue();
    ~Queue();
    void enqueue(Interaction interaction);
    Interaction dequeue();
    Interaction peek();
    bool isEmpty();
    int getSize();
    void display();
};

#endif // QUEUE_H
