#ifndef INTERACTION_MANAGER_H
#define INTERACTION_MANAGER_H

#include "../data_structures/Queue.h"
#include "Item.h"
#include "User.h"

class InteractionManager {
public:
  
    // The "API" endpoint for the frontend to send data
    static void sendToAPI(Queue& q, int uID, int iID, InteractionType type);

    // The backend process that updates scores
    static void processAll(Queue& q, Item& targetItem, User& activeUser);

    // Database operations
    static void loadFromFile(Queue& q);
    static void logToFile(const Interaction& i);
    static void recordInteraction(Queue& q, int uID, int iID, InteractionType type);




};

#endif