#include "Item.h"
#include "User.h"
#include "Vector.h"
#include <iostream>
#include <string>
using namespace std;

Vector<ProductDTO> products;

Vector<CartItem> userCarts[1000];

int next_product_id = 1;

int total_orders = 0;

double total_revenue = 0;

int total_users = 0;


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


Vector<ProductDTO>
Item::list_products(const string& category)
{
    Vector<ProductDTO> result;

    for(int i = 0; i < products.getSize(); i++)
    {
        ProductDTO p = products.get(i);

        if(category == "" || p.category == category)
        {
            result.push_back(p);
        }
    }

    return result;
}

ProductDTO
Item::get_product_by_id(int item_id)
{
    ProductDTO empty;

    empty.item_id = -1;

    for(int i = 0; i < products.getSize(); i++)
    {
        ProductDTO p = products.get(i);

        if(p.item_id == item_id)
        {
            return p;
        }
    }

    return empty;
}

ProductDTO
Item::create_product(const ProductInput& input)
{
    ProductDTO product;

    product.item_id = next_product_id;

    next_product_id++;

    product.name = input.name;

    product.category = input.category;

    product.price = input.price;

    product.stock = input.stock;

    products.push_back(product);

    return product;
}

ProductDTO
Item::update_product(int item_id, const ProductInput& input)
{
    ProductDTO empty;

    empty.item_id = -1;

    for(int i = 0; i < products.getSize(); i++)
    {
        ProductDTO p = products.get(i);

        if(p.item_id == item_id)
        {
            p.name = input.name;

            p.category = input.category;

            p.price = input.price;

            p.stock = input.stock;

            products.set(i, p);

            return p;
        }
    }

    return empty;
}

bool Item::delete_product(int item_id)
{
    for(int i = 0; i < products.getSize(); i++)
    {
        ProductDTO p = products.get(i);

        if(p.item_id == item_id)
        {
            products.remove(i);

            return true;
        }
    }

    return false;
}

Vector<CartItemDTO>
Item::get_cart(int user_id)
{
    Vector<CartItemDTO> result;

    for(int i = 0; i < userCarts[user_id].getSize(); i++)
    {
        CartItem cartItem = userCarts[user_id].get(i);

        ProductDTO product = get_product_by_id(cartItem.item_id);

        if(product.item_id != -1)
        {
            CartItemDTO dto;

            dto.item_id = product.item_id;

            dto.name = product.name;

            dto.quantity = cartItem.quantity;

            dto.price = product.price;

            result.push_back(dto);
        }
    }

    return result;
}

bool Item::add_to_cart(int user_id, int item_id, int quantity)
{
    ProductDTO product =
        get_product_by_id(item_id);

    if(product.item_id == -1)
    {
        return false;
    }

    if(product.stock < quantity)
    {
        return false;
    }

    for(int i = 0; i < userCarts[user_id].getSize(); i++)
    {
        CartItem cartItem = userCarts[user_id].get(i);

        if(cartItem.item_id == item_id)
        {
            cartItem.quantity += quantity;

            userCarts[user_id].set(i, cartItem);

            return true;
        }
    }

    CartItem newItem;

    newItem.item_id = item_id;

    newItem.quantity = quantity;

    userCarts[user_id].push_back(newItem);

    return true;
}

bool Item::remove_from_cart(int user_id, int item_id)
{
    for(int i = 0; i < userCarts[user_id].getSize(); i++)
    {
        CartItem cartItem = userCarts[user_id].get(i);

        if(cartItem.item_id == item_id)
        {
            userCarts[user_id].remove(i);

            return true;
        }
    }

    return false;
}

CheckoutResult
Item::checkout(int user_id)
{
    CheckoutResult result;

    result.success = false;

    result.total = 0;

    if(userCarts[user_id].isEmpty())
    {
        result.message = "Cart is empty";

        return result;
    }

    double total = 0;

    for(int i = 0; i < userCarts[user_id].getSize(); i++)
    {
        CartItem cartItem = userCarts[user_id].get(i);

        ProductDTO product = get_product_by_id(cartItem.item_id);

        if(product.stock < cartItem.quantity)
        {
            result.message = "Insufficient stock";

            return result;
        }
    }

    for(int i = 0; i < userCarts[user_id].getSize(); i++)
    {
        CartItem cartItem = userCarts[user_id].get(i);

        for(int j = 0; j < products.getSize(); j++)
        {
            ProductDTO product = products.get(j);

            if(product.item_id == cartItem.item_id)
            {
                product.stock -= cartItem.quantity;

                total += product.price * cartItem.quantity;

                products.set(j, product);
            }
        }
    }

    total_orders++;

    total_revenue += total;

    while(!userCarts[user_id].isEmpty())
    {
        userCarts[user_id].pop_back();
    }

    result.success = true;

    result.total = total;

    result.message = "Checkout successful";

    return result;
}

DashboardStats
Item::get_dashboard_stats()
{
    DashboardStats stats;

    stats.total_products = products.getSize();

    stats.total_users = total_users;

    stats.total_orders = total_orders;

    stats.total_revenue = total_revenue;

    return stats;
}