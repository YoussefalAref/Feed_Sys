# Architecture Overview — Smart E-Commerce Recommendation Platform

## Project Purpose

A C++ learning project that builds a recommendation feed for an e-commerce platform
using **only custom data structures** (no STL containers).

---

## System Layers

```
┌─────────────────────────────────────────────────────────────┐
│                        main.cpp  (Driver)                   │
│  Creates data → simulates interactions → displays results   │
└──────────────────────────┬──────────────────────────────────┘
                           │ uses
        ┌──────────────────┼──────────────────────┐
        │                  │                       │
        ▼                  ▼                       ▼
┌──────────────┐  ┌───────────────┐   ┌─────────────────────┐
│   HashMap    │  │    Heap       │   │   Queue             │
│  (Users +    │  │  (Ranking)    │   │  (Activity Tracker) │
│   Items)     │  │               │   │                     │
└──────────────┘  └───────────────┘   └─────────────────────┘
        │                                          │
        │           ┌──────────────┐               │
        └──────────►│    Graph     │◄──────────────┘
                    │  (Similarity)│
                    └──────────────┘
```

---

## Data Structures Used and Why

| Data Structure | File | Role | Why This DS? |
|---|---|---|---|
| **HashMap** | `HashMap.h` | Store users and items for O(1) lookup | Fast login, fast item retrieval |
| **MaxHeap** | `Heap.h` | Rank items by popularity score | Always have the top item in O(1), insert/remove in O(log n) |
| **Queue** | `Queue.h` | Record interactions in arrival order (FIFO) | Chronological processing, O(1) enqueue/dequeue |
| **Graph** | `Graph.h` | Model item similarity relationships | Discover "you might also like" neighbours |

---

## Models

```
User
 ├── userID         (int)
 ├── username       (string)
 ├── email          (string)
 ├── password       (string)
 ├── activityScore  (int)
 └── userLevel      (string: "Normal" | "Active" | "VIP")

Item
 ├── itemID          (int)
 ├── name            (string)
 ├── category        (string)
 ├── price           (double)
 ├── popularityScore (double)
 └── stock           (int)

Interaction
 ├── userID     (int)
 ├── itemID     (int)
 ├── type       (enum: VIEW | CLICK | ADD_TO_CART | PURCHASE)
 └── timestamp  (long)
```

---

## Recommendation Flow

```
User opens the app
        │
        ▼
   Is this user NEW?
   ┌────┴─────┐
  YES         NO
   │           │
   ▼           ▼
Global     Build temporary heap with
Heap   →   personalised score:
Top-K      popularity + similarity + interestBoost
Items      Then extract top-K items
```

### Personalised Score Formula (Milestone 2)
```
score(item, user) =
    item.popularityScore
  + graphSimilarity(item, user's recently viewed items)
  + userInterestBoost(item.category, user.history)
```

---

## Interaction Scoring

| Event | Popularity Score Added | User Activity Added |
|---|---|---|
| VIEW | +1 | +1 |
| CLICK | +2 | +2 |
| ADD_TO_CART | +5 | +5 |
| PURCHASE | +10 | +10 |

### User Level Thresholds
```
activityScore > 1000  →  VIP
activityScore >  500  →  Active
else                  →  Normal
```

---

## UML Class Diagram (Simplified)

```
┌─────────────────────┐     uses     ┌─────────────────────┐
│  HashMap<int,User>  │◄─────────────│      main.cpp       │
└─────────────────────┘              │                     │
                                     │  UserHashMap        │
┌─────────────────────┐              │  ItemHashMap        │
│  HashMap<int,Item>  │◄─────────────│  Heap               │
└─────────────────────┘              │  Queue              │
                                     │  Graph              │
┌─────────────────────┐              └──────────┬──────────┘
│  Heap (MaxHeap)     │◄────────────────────────┤
│  stores: Item[]     │                         │
└─────────────────────┘                         │
                                                 │
┌─────────────────────┐                         │
│  Queue              │◄────────────────────────┤
│  stores: Interaction│                         │
└─────────────────────┘                         │
                                                 │
┌─────────────────────┐                         │
│  Graph              │◄────────────────────────┘
│  nodes: itemID      │
│  edges: similarity  │
└─────────────────────┘
```

---

## File Structure

```
Feed_Sys/
├── src/
│   ├── main.cpp                        ← entry point (Developer 5)
│   ├── models/
│   │   ├── User.h                      ← User model  (Developer 5)
│   │   ├── Item.h                      ← Item model  (Developer 5)
│   │   └── Interaction.h               ← Interaction model (Developer 5)
│   └── data_structures/
│       ├── HashMap.h                   ← Generic HashMap  (Developer 1)
│       ├── Heap.h                      ← MaxHeap          (Developer 2)
│       ├── Queue.h                     ← Activity Queue   (Developer 3)
│       └── Graph.h                     ← Similarity Graph (Developer 4)
├── docs/
│   └── architecture.md                 ← this file
├── .gitignore
└── README.md
```

---

## Developer Assignments

| Developer | Module | Key DS | Files |
|---|---|---|---|
| Developer 1 | HashMap | Hash Map + Linked List | `HashMap.h` |
| Developer 2 | Heap | Binary Heap (array) | `Heap.h` |
| Developer 3 | Queue | Linked List Queue | `Queue.h` |
| Developer 4 | Graph | Adjacency List | `Graph.h` |
| Developer 5 | Integration + Models | All | `User.h`, `Item.h`, `Interaction.h`, `main.cpp` |

---

## Complexity Summary

| Operation | HashMap | Heap | Queue | Graph |
|---|---|---|---|---|
| Insert | O(1) avg | O(log n) | O(1) | O(1) |
| Search/Peek | O(1) avg | O(1) | O(1) | O(deg) |
| Delete/Extract | O(1) avg | O(log n) | O(1) | O(deg) |
| Space | O(n) | O(n) | O(n) | O(V+E) |

---

## Milestones

### Milestone 1 (Current) — Data Structures
- Implement all four data structures from scratch (no STL)
- Define models (User, Item, Interaction)
- Demo: insert users/items, simulate interactions, display rankings

### Milestone 2 — Recommendation Engine
- Scoring function: popularity + similarity + interest boost
- Time decay for old interactions
- Personalised heap built per request

### Final System — Full Platform
- Web UI
- Authentication system (using HashMap)
- Shopping cart
- Payment integration (Paymob)
- Graph-based recommendations
