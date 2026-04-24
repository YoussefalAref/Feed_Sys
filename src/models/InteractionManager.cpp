#include "InteractionManager.h"
#include <iostream>
#include <fstream>

void InteractionManager::sendToAPI(Queue& q, int uID, int iID, InteractionType type) {
    Interaction newEvent(uID, iID, type);
    q.enqueue(newEvent);
    std::cout << "[Frontend] Event sent to API: ";
    newEvent.display();
    //NEEDS MODIFICATION.............
}

void InteractionManager::processAll(Queue& q, Item& targetItem, User& activeUser) {
    while (!q.isEmpty()) {
        Interaction i = q.dequeue(); // FIFO Recency Tracking
        InteractionType type = i.getType();

        // 1. Update Item Popularity
        targetItem.updateScore(type); //DEV2 handles this logic based on interaction type

        // 2. Update User Activity (using weights from Interaction.h)
        if (type == VIEW) activeUser.activityScore += 1;//DEV1 handles this logic based on interaction type
        else if (type == CLICK) activeUser.activityScore += 2;
        else if (type == ADD_TO_CART) activeUser.activityScore += 5;
        else if (type == PURCHASE) activeUser.activityScore += 10;

        activeUser.updateLevel();//DEV1 handles this logic based on activity score thresholds
    }
}
// DATABASE RESPONSIBILITY: Store interaction logs in a text file
void InteractionManager::logToFile(const Interaction& i) {
    // Open in append mode so we don't overwrite previous logs
    std::ofstream dbFile("data/interactions_db.txt", std::ios::app);
    if (dbFile.is_open()) {
        dbFile << i.getUserID() << " " 
               << i.getItemID() << " " 
               << i.getType() << " " 
               << i.getTimestamp() << "\n";
        dbFile.close();
    }
}
void InteractionManager::recordInteraction(Queue& q, int uID, int iID, InteractionType type) {
    //Create the Interaction object
    Interaction newEvent(uID, iID, type);

    //DATABASE: Store log in text file (Persistence)
    logToFile(newEvent);
    // 3. API/QUEUE: Add to the recency tracker
    q.enqueue(newEvent);
}
void InteractionManager::loadFromFile(Queue& q) {
    std::ifstream dbFile("data/interactions_db.txt");
    if (!dbFile.is_open()) {
        std::cout << "No saved interactions found." << std::endl;
        return;
    }

    int uID, iID, typeInt;
    long timestamp;

    while (dbFile >> uID >> iID >> typeInt >> timestamp) {
        InteractionType type = static_cast<InteractionType>(typeInt);
        Interaction i(uID, iID, type, timestamp);
        q.enqueue(i);
    }

    dbFile.close();
    std::cout << "Loaded interactions from database." << std::endl;
}
