#include "data_structures/Graph.h"
#include "data_structures/HashMap.h"
#include "data_structures/Heap.h"
#include "data_structures/Queue.h"
#include "models/Interaction.h"
#include "models/Item.h"
#include "models/User.h"

#include <iostream>
#include <string>

using namespace std;

const int USER_COUNT = 5;
const int ITEM_COUNT = 10;

int interactionWeight(InteractionType type) {
    if (type == VIEW) return 1;
    if (type == CLICK) return 2;
    if (type == ADD_TO_CART) return 5;
    if (type == PURCHASE) return 10;
    return 0;
}

int findItemIndex(Item items[], int count, int itemId) {
    string id = to_string(itemId);
    for (int i = 0; i < count; ++i) {
        if (items[i].getID() == id) return i;
    }
    return -1;
}

int findUserIndex(User users[], int count, int userId) {
    for (int i = 0; i < count; ++i) {
        if (users[i].getUserId() == userId) return i;
    }
    return -1;
}

void applyUserActivity(User& user, InteractionType type) {
    if (type == VIEW) user.addViews();
    else if (type == CLICK) user.addClicks();
    else if (type == ADD_TO_CART) user.addCartAdds();
    else if (type == PURCHASE) user.addPurchases();
}

void enqueueDemoInteractions(Queue& interactions) {
    interactions.enqueue(Interaction(1, 101, VIEW));
    interactions.enqueue(Interaction(1, 106, CLICK));
    interactions.enqueue(Interaction(1, 106, ADD_TO_CART));
    interactions.enqueue(Interaction(1, 101, PURCHASE));
    interactions.enqueue(Interaction(2, 102, VIEW));
    interactions.enqueue(Interaction(2, 107, ADD_TO_CART));
    interactions.enqueue(Interaction(2, 102, PURCHASE));
    interactions.enqueue(Interaction(3, 108, VIEW));
    interactions.enqueue(Interaction(3, 103, CLICK));
    interactions.enqueue(Interaction(4, 104, VIEW));
    interactions.enqueue(Interaction(4, 104, PURCHASE));
    interactions.enqueue(Interaction(5, 105, VIEW));
    interactions.enqueue(Interaction(5, 110, CLICK));
    interactions.enqueue(Interaction(5, 105, ADD_TO_CART));
    interactions.enqueue(Interaction(1, 106, PURCHASE));
}

void processInteractions(Queue& queue, Item items[], int itemCount, User users[], int userCount) {
    cout << "\nProcessing FIFO interaction queue\n";
    while (!queue.isEmpty()) {
        Interaction event = queue.dequeue();
        int weight = interactionWeight(event.getType());

        int itemIndex = findItemIndex(items, itemCount, event.getItemID());
        if (itemIndex >= 0) {
            items[itemIndex].incrementPopularityScore(weight);
        }

        int userIndex = findUserIndex(users, userCount, event.getUserID());
        if (userIndex >= 0) {
            applyUserActivity(users[userIndex], event.getType());
        }

        event.display();
    }
}

void showTopItems(Item items[], int itemCount, int limit) {
    Heap heap(itemCount);
    for (int i = 0; i < itemCount; ++i) {
        heap.insert(items[i]);
    }

    cout << "\nTop " << limit << " products by heap ranking\n";
    for (int i = 0; i < limit && !heap.isEmpty(); ++i) {
        Item item = heap.extractMax();
        cout << i + 1 << ". " << item.getName()
             << " | score=" << item.getPopularityScore()
             << " | category=" << item.getCategory() << "\n";
    }
}

void showUserLevels(User users[], int userCount) {
    cout << "\nUser activity levels\n";
    for (int i = 0; i < userCount; ++i) {
        cout << "User " << users[i].getUserId()
             << " | " << users[i].getEmail()
             << " | activity=" << users[i].getActivityScore()
             << " | level=" << users[i].getUserLevel() << "\n";
    }
}

void showGraphNeighbors(Graph& graph, const string& itemId) {
    int count = 0;
    string* neighbors = graph.getNeighbors(itemId, count);

    cout << "\nGraph neighbors for item " << itemId << "\n";
    if (count == 0 || neighbors == nullptr) {
        cout << "No related products found.\n";
        return;
    }

    for (int i = 0; i < count; ++i) {
        cout << "- " << neighbors[i] << "\n";
    }
}

void showPersonalizedCandidates(User& user, Item items[], int itemCount, Graph& graph) {
    Heap personalized(itemCount);

    for (int i = 0; i < itemCount; ++i) {
        Item candidate = items[i];
        int score = candidate.getPopularityScore();

        if (candidate.getCategory() == user.getCategory()) {
            score += 15;
        }

        int neighborCount = 0;
        string* neighbors = graph.getNeighbors(candidate.getID(), neighborCount);
        if (neighbors != nullptr && neighborCount > 0) {
            score += 5;
        }

        candidate.setPopularityScore(score);
        personalized.insert(candidate);
    }

    cout << "\nPersonalized top 3 for " << user.getEmail()
         << " using popularity + category boost + graph signal\n";
    for (int i = 0; i < 3 && !personalized.isEmpty(); ++i) {
        Item item = personalized.extractMax();
        cout << i + 1 << ". " << item.getName()
             << " | recommendationScore=" << item.getPopularityScore() << "\n";
    }
}

int main() {
    User users[USER_COUNT] = {
        User(1, "mariam@biteapple.test", "hash-1", "Electronics", 92),
        User(2, "omar@biteapple.test", "hash-2", "Fitness", 76),
        User(3, "manager@biteapple.test", "hash-3", "Business", 98),
        User(4, "nour@biteapple.test", "hash-4", "Fashion", 61),
        User(5, "salma@biteapple.test", "hash-5", "Kitchen", 70),
    };

    Item items[ITEM_COUNT] = {
        Item("101", "AirPods Pro MaxCase", 2499, "Electronics", 94, 18),
        Item("102", "Smart Fitness Band", 1299, "Fitness", 87, 31),
        Item("103", "Minimal Desk Lamp", 850, "Home", 78, 24),
        Item("104", "Campus Backpack", 1100, "Fashion", 83, 15),
        Item("105", "Cold Brew Kit", 690, "Kitchen", 72, 22),
        Item("106", "Wireless Keyboard", 1650, "Electronics", 91, 12),
        Item("107", "Yoga Recovery Mat", 780, "Fitness", 69, 27),
        Item("108", "Ceramic Dinner Set", 1850, "Home", 81, 9),
        Item("109", "Running Shoes", 1450, "Fitness", 74, 16),
        Item("110", "Espresso Grinder", 2100, "Kitchen", 79, 8),
    };

    HashMap<int, string> userEmailLookup;
    HashMap<int, string> itemNameLookup;
    for (int i = 0; i < USER_COUNT; ++i) {
        userEmailLookup.insert(users[i].getUserId(), users[i].getEmail());
    }
    for (int i = 0; i < ITEM_COUNT; ++i) {
        itemNameLookup.insert(stoi(items[i].getID()), items[i].getName());
    }

    cout << "HashMap lookup demo\n";
    cout << "User 1 email: " << userEmailLookup.search(1) << "\n";
    cout << "Item 106 name: " << itemNameLookup.search(106) << "\n";

    Graph graph;
    graph.build(items, ITEM_COUNT);

    Queue interactionQueue;
    enqueueDemoInteractions(interactionQueue);
    cout << "\nQueued interactions: " << interactionQueue.getSize() << "\n";

    processInteractions(interactionQueue, items, ITEM_COUNT, users, USER_COUNT);

    showTopItems(items, ITEM_COUNT, 5);
    showUserLevels(users, USER_COUNT);
    showGraphNeighbors(graph, "101");
    showPersonalizedCandidates(users[0], items, ITEM_COUNT, graph);

    return 0;
}
