#ifndef ITEM_H
#define ITEM_H
#include <iostream>
#include <string>
using namespace std;


class Item{
private:
string ID;
string name;
double price;
string category;
int popularityScore;
int stock;

public:
Item(string id, string n, double p, string c, int pS, int s);
string getID() const;
string getName() const;
double getPrice() const;
string getCategory() const;
int getPopularityScore() const;
int getStock() const;
void setID(string id);
void setName(string n);
void setPrice(double p);
void setCategory(string c);
void setPopularityScore(int ps);
void setStock(int s);
void incrementPopularityScore(int num);
void resetPopularity();
void displayItem() const;
};
#endif
