#include "Item.h"
#include <iostream>
#include <string>
using namespace std;

Item::Item(string id, string n, double p, string c, int pS, int s){
   ID = id;
   name = n;
   price = p;
   category = c;
   popularityScore = pS;
   stock = s;
}

string Item::getID() const{
    return ID;
}

string Item:: getName() const{
    return name;
}

double Item::getPrice() const{
    return price;
}

string Item::getCategory() const{
    return category;
}

int Item::getPopularityScore() const{
    return popularityScore;
}
int Item::getStock() const{
    return stock;
}

void Item::setID(string id){
    ID = id;
}

void Item::setName(string n){
    name = n;
}

void Item::setPrice(double p){
    price = p;
}

void Item::setCategory(string c){
    category = c;
}

void Item::setPopularityScore(int ps){
    popularityScore = ps;
}

void Item::setStock(int s){
    stock = s;
}

void Item::incrementPopularityScore(int num){
    popularityScore += num;
}
void Item::decrementPopularityScore(int num){
    popularityScore -= num;
}
void Item::resetPopularity(){
    popularityScore = 0;
}

void Item::displayItem() const{
    cout<<"Item's ID is: "<<ID<<endl;
    cout<<"Item's Name is: "<<name<<endl;
    cout<<"Item's price is: "<<price<<endl;
    cout<<"Item's category is: "<<category<<endl;
    cout<<"Item's popularity score is: "<<popularityScore<<endl;
    cout<<"Number of items in stock is: "<<stock<<endl;
}
