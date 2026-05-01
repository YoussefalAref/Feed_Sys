#include "Authentication.h"
#include <iostream>

// Initialise the ID counter. Seed this from the highest stored ID on startup
// if users are loaded from a persistent database.
int Authentication::nextUserId_ = 1;

// ---------------------------------------------------------------------------
// findByEmail  (private helper)
//
// HashMap is keyed by userId (int), so there is no O(1) path for email.
// We iterate every bucket and walk each chain — O(n) over stored users.
// This is acceptable: authentication is infrequent and n (registered users
// in memory) is small compared to the cost of a network round-trip.
//
// Returns a raw pointer to the value inside the bucket node, or nullptr.
// The pointer is valid only as long as the HashMap entry is not removed
// or rehashed — callers must copy the User before returning it externally.
// ---------------------------------------------------------------------------
User* Authentication::findByEmail(HashMap<int, User>& users,
                                  const std::string& email) {
    for (int i = 0; i < users.getCapacity(); ++i) {
        // Walk the chained linked list at this bucket
        const Node<int, User>* node = users.getBucket(i);
        while (node) {
            // const_cast is safe: we only write through this pointer in
            // findByEmail's callers when they explicitly intend mutation.
            if (node->value.getEmail() == email)
                return const_cast<User*>(&node->value);
            node = node->next;
        }
    }
    return nullptr; // email not registered
}

// ---------------------------------------------------------------------------
// authenticate
//
// 1. Scan the HashMap for a user whose email matches (O(n) bucket walk).
// 2. Compare the stored password hash against the supplied hash.
// 3. Return a heap-allocated copy on success so the caller owns the object
//    and the HashMap's internal state is never exposed directly.
// ---------------------------------------------------------------------------
User* Authentication::authenticate(HashMap<int, User>& users,
                                   const std::string& email,
                                   const std::string& passwordHash) {
    User* found = findByEmail(users, email);

    if (!found) {
        std::cout << "[Auth] Login failed: email not found (" << email << ")\n";
        return nullptr;
    }

    if (found->getPasswordHash() != passwordHash) {
        std::cout << "[Auth] Login failed: wrong password for " << email << "\n";
        return nullptr;
    }

    std::cout << "[Auth] Login success: " << email << "\n";
    return new User(*found); // caller owns this copy
}

// ---------------------------------------------------------------------------
// createUser
//
// 1. Reject duplicate emails by scanning the bucket chains (O(n)).
// 2. Assign a monotonically increasing userId — guaranteed unique within
//    this session; sync nextUserId_ with the DB max on startup for persistence.
// 3. insert() into the HashMap using userId as the key — O(1) average.
// 4. Re-fetch from the map via search() and return a heap copy so the caller
//    gets a stable pointer unaffected by future rehashes.
// ---------------------------------------------------------------------------
User* Authentication::createUser(HashMap<int, User>& users,
                                 const std::string& email,
                                 const std::string& passwordHash,
                                 const std::string& category) {
    if (findByEmail(users, email)) {
        std::cout << "[Auth] Signup failed: email already registered (" << email << ")\n";
        return nullptr; // duplicate email — no insert
    }

    int  newId = nextUserId_++;
    User newUser(newId, email, passwordHash, category, 0.0f);

    // O(1) average insert; triggers rehash internally if load factor exceeded
    users.insert(newId, newUser);

    // search() returns V() (invalid User) on miss, so validate before returning
    User fetched = users.search(newId);
    if (!fetched.isValid()) {
        std::cout << "[Auth] Internal error: user insert failed for id " << newId << "\n";
        return nullptr;
    }

    std::cout << "[Auth] Signup success: " << email << " (id=" << newId << ")\n";
    return new User(fetched); // caller owns this copy
}

// ---------------------------------------------------------------------------
// getUserById
//
// Direct O(1) average lookup by the HashMap key.
// search() returns a default-constructed User (isValid() == false) on miss.
// ---------------------------------------------------------------------------
User* Authentication::getUserById(HashMap<int, User>& users, int userId) {
    User found = users.search(userId); // O(1) average

    if (!found.isValid()) {
        std::cout << "[Auth] getUserById: no user with id " << userId << "\n";
        return nullptr;
    }

    return new User(found); // caller owns this copy
}

// ---------------------------------------------------------------------------
// getUserCategory
//
// Thin wrapper around getUserById — returns the category string or "".
// Does NOT return a heap object so the caller has nothing to delete.
// ---------------------------------------------------------------------------
std::string Authentication::getUserCategory(HashMap<int, User>& users, int userId) {
    User found = users.search(userId);

    if (!found.isValid()) {
        std::cout << "[Auth] getUserCategory: no user with id " << userId << "\n";
        return "";
    }

    return found.getCategory();
}

// ---------------------------------------------------------------------------
// updateUserScore
//
// 1. Verify the user exists via search().
// 2. Mutate the local copy's score.
// 3. Re-insert (upsert) back into the HashMap to persist the change.
//    update() would also work here but insert() is safer: it handles
//    the unlikely edge case where the key was removed between the search
//    and this call (it would re-add rather than throw).
// ---------------------------------------------------------------------------
bool Authentication::updateUserScore(HashMap<int, User>& users,
                                     int userId, float newScore) {
    User found = users.search(userId);

    if (!found.isValid()) {
        std::cout << "[Auth] updateUserScore: no user with id " << userId << "\n";
        return false;
    }

    found.setScore(newScore);
    users.insert(userId, found); // upsert — O(1) average
    return true;
}
