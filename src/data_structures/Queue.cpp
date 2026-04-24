#include "Queue.h"
#include<iostream>
#include <stdexcept>
    Queue::Queue() : front(nullptr), back(nullptr), size(0) {}
    Queue::~Queue() {
        while (front != nullptr) {
            Node* temp = front;
            front = front->next;
            delete temp;
        }
    }
    void Queue:: enqueue(Interaction interaction){
        Node* newnode = new Node(interaction);
        if(back==nullptr){
            front = back =newnode;
        } else {
            back->next = newnode;
            back = newnode;
        }
        size++;   
    }
    Interaction Queue:: dequeue(){
        if(back==nullptr){
            throw std::runtime_error("Queue is empty");
        }
        Interaction x=front->data;
        Node* temp=front;
        front=front->next;
        if(front==nullptr){
            back=nullptr;
        }
        delete temp;
        size--;
        return x;
    }
    Interaction Queue:: peek(){
        if(back==nullptr){
            throw std::runtime_error("Queue is empty");
        }
        return front->data;
    }
    bool Queue:: isEmpty(){
        if(size==0){
            return true;
        }
        return false;
        }
    int Queue::getSize(){
        return size;
    }
    void Queue:: display(){
        if(back==nullptr){
            std::cout<<"Queue is empty"<<std::endl;
            return;
        }
        Node* temp=front;
        while(temp!=nullptr){
            temp->data.display();
            temp=temp->next;
        }
        temp=nullptr;;
    }