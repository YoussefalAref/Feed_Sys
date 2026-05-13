
#ifndef INTERACTION_MANAGER_H
#define INTERACTION_MANAGER_H

#include "Queue.h"
#include "Item.h"
#include "User.h"
#include "Vector.h"
#include "DTOs.h"
#include "DataStore.h"
std::string intToString(int n);

class InteractionManager {
    Vector<Interaction> history; // Keep a history of all interactions 
public:
  
    // The "API" endpoint for the frontend to send data
    static void sendToAPI(Queue& q, int uID, int iID, InteractionType type);

    // The backend process that updates scores
    static void processAll(Queue& q, Item& targetItem, User& activeUser);

    // Database operations
    static void loadFromFile(Queue& q);
    static void logToFile(const Interaction& i);
    static void record_interaction(Queue& q, int uID, int iID, InteractionType type);
    Queue interactionQueue;
    InteractionDTO record_interaction(int user_id, int item_id, const std::string& type);
    Vector<InteractionDTO> get_recent_interactions(int limit);


};

#endif