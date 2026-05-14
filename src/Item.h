#ifndef ITEM_H
#define ITEM_H
#include <iostream>
#include <string>
#include "Vector.h"
#include "DTOs.h"
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
Item() = default;
Item(const std::string& id, const std::string& name, int popularityScore);
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


    // product functions
    static Vector<ProductDTO> list_products(const string& category);

    static ProductDTO get_product_by_id(int item_id);

    static ProductDTO create_product(const ProductInput& input);

    static ProductDTO update_product(int item_id, const ProductInput& input);

    static bool delete_product(int item_id);

    // cart functions
    static Vector<CartItemDTO> get_cart(int user_id);

    static bool add_to_cart(int user_id, int item_id, int quantity);

    static bool remove_from_cart(int user_id, int item_id);

    static CheckoutResult checkout(int user_id);

    // dashboard
    static DashboardStats get_dashboard_stats();

};
#endif