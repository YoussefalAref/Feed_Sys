#include "Interaction.h"
#include <iostream>
    Interaction::Interaction(int userID, int itemID, InteractionType type) : userID(userID), itemID(itemID), type(type) {
        timestamp = std::time(nullptr); // Current time in seconds since epoch
    }
    //for saving in file
    Interaction::Interaction(int userID, int itemID, InteractionType type, long timestamp) : userID(userID), itemID(itemID), type(type), timestamp(timestamp) {}
    int Interaction::getUserID() const {
        return userID;
    }
    int Interaction::getItemID() const {
        return itemID;
    }
    InteractionType Interaction::getType() const {
        return type;
    }
    long Interaction::getTimestamp() const {
        return timestamp;
    }
    void Interaction::display() const {
        std::string typeStr;
        switch (type) {
            case VIEW: typeStr = "VIEW"; break;
            case CLICK: typeStr = "CLICK"; break;
            case ADD_TO_CART: typeStr = "ADD_TO_CART"; break;
            case PURCHASE: typeStr = "PURCHASE"; break;
        }
        std::cout << "User " << userID << " " << typeStr << " item " << itemID << " at time " << timestamp << std::endl;
        return;
    }