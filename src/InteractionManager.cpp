#include "InteractionManager.h"
#include <iostream>
#include <fstream>

Vector<InteractionDTO> InteractionManager::get_recent_interactions(int limit) {
    Vector<InteractionDTO> recent;
    int start = (history.getSize() > limit) ? (history.getSize() - limit) : 0;

    // Get the last 'limit' items from the history vector
    for (int i = history.getSize() - 1; i >= start; i--) {
        Interaction iObj = history.get(i);
        
        InteractionDTO dto;
        dto.user_id = iObj.getUserID();
        dto.item_id = iObj.getItemID();

        if (iObj.getType() == VIEW) dto.type = "view";
        else if (iObj.getType() == CLICK) dto.type = "click";
        else if (iObj.getType() == ADD_TO_CART) dto.type = "cart";
        else if (iObj.getType() == PURCHASE) dto.type = "purchase";
        
        dto.timestamp = intToString(iObj.getTimestamp());
        
        recent.push_back(dto);
    }
    return recent; 
}


// DATABASE RESPONSIBILITY: Store interaction logs in a text file
void InteractionManager::logToFile(const Interaction& i) {
    // Open in append mode so we don't overwrite previous logs
    std::ofstream dbFile("../data/interactions_db.txt", std::ios::app);
    if (dbFile.is_open()) {
        dbFile << i.getUserID() << " " 
               << i.getItemID() << " " 
               << i.getType() << " " 
               << i.getTimestamp() << "\n";
        dbFile.close();
    }
}


std::string intToString(int n) {
    if (n == 0) return "0";
    char buf[12];
    int i = 11;
    buf[i] = '\0';
    while (n > 0) {
        buf[--i] = '0' + (n % 10);
        n /= 10;
    }
    return std::string(buf + i);
}

InteractionDTO InteractionManager::record_interaction(int user_id, int item_id, const string& interaction_type) {
    // Convert int item_id to string for HashMap lookup
    string itemKey = intToString(item_id);
    // 1. Validate item exists
    Item foundItem = itemStore.search(itemKey);
    if (foundItem.getID() == "") {
        std::cerr << "[Error] Item ID " << item_id << " not found. Aborting interaction." << std::endl;
        InteractionDTO errorDto;
        errorDto.user_id = -1; // Flag as invalid
        return errorDto;
    }

    // 2. Validate user exists — confirm with DEV1 what their lookup returns
    User foundUser = userStore.search(user_id); 
    if (!foundUser.isValid()) {
        std::cerr << "[Error] User ID " << user_id << " not found. Aborting interaction." << std::endl;
        InteractionDTO errorDto;
        errorDto.user_id = -1; // Flag as invalid
        return errorDto;
    }

    // 3. Convert string type to enum
    InteractionType type;
    if      (interaction_type == "view")     type = VIEW;
    else if (interaction_type == "click")    type = CLICK;
    else if (interaction_type == "cart")     type = ADD_TO_CART;
    else if (interaction_type == "purchase") type = PURCHASE;

    // 4. Create interaction object
    Interaction newEvent(user_id, item_id, type);

    // 5. Log to file
    logToFile(newEvent);

    // 6. Enqueue into the member queue (not a local — persists on the instance)
    interactionQueue.enqueue(newEvent);
    history.push_back(newEvent);

    // 7. Drain the queue and apply score effects
    while (!interactionQueue.isEmpty()) {
        Interaction event = interactionQueue.dequeue();
        InteractionType evType = event.getType();

        if      (evType == VIEW)         foundItem.incrementPopularityScore(1);
        else if (evType == CLICK)        foundItem.incrementPopularityScore(2);
        else if (evType == ADD_TO_CART)  foundItem.incrementPopularityScore(5);
        else if (evType == PURCHASE)     foundItem.incrementPopularityScore(10);
        itemStore.insert(itemKey, foundItem);

        if      (evType == VIEW)         foundUser.addViews(1);
        else if (evType == CLICK)        foundUser.addClicks(1);
        else if (evType == ADD_TO_CART)  foundUser.addCartAdds(1);
        else if (evType == PURCHASE)     foundUser.addPurchases(1);
        userStore.insert(user_id, foundUser);
    }

    // 9. Build and return DTO
    InteractionDTO dto;
    dto.user_id = user_id;
    dto.item_id = item_id;
    dto.type    = interaction_type;
    // timestamp conversion to ISO string
    dto.timestamp = ""; // fill in
    return dto;
}
