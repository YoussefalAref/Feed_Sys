#ifndef STATE_STRUCTS_H
#define STATE_STRUCTS_H

#include <string>

// Forward declarations and state structs for the bindings
struct UserState {
    int id = -1;
    std::string name;
    std::string email;
    std::string password;
    std::string category;
    std::string region;
    double score = 50.0;
    int activity_score = 0;
    std::string level = "Normal";
};

struct ProductState {
    int id = -1;
    std::string name;
    double price = 0.0;
    std::string category;
    double popularity_score = 0.0;
    int stock = 0;
    std::string image;
    std::string description;
};

struct CartEntry {
    int user_id = -1;
    int item_id = -1;
    int quantity = 0;
};

struct InteractionState {
    int id = -1;
    int user_id = -1;
    int item_id = -1;
    std::string type;
    std::string timestamp;
};

#endif
