#ifndef Authentication_h
#define Authentication_h

#include "HashMap.h"
#include "User.h"
#include <string>

/*
 * Authentication — stateless utility class for user identity operations.
 *
 * All methods are static; there is no Authentication instance to manage.
 *
 * HashMap usage notes:
 *   - The primary store is HashMap<int, User>, keyed by userId.
 *   - search(userId) runs in O(1) average — used by getId-based lookups.
 *   - Email lookups have no direct key, so they scan all buckets via
 *     getBucket() — O(n) worst case, acceptable for auth (infrequent).
 *   - insert() is an upsert; createUser guards duplicates before calling it.
 *
 * Ownership of returned User*:
 *   All returned pointers point to heap-allocated COPIES of the stored User.
 *   The caller is responsible for deleting them when done.
 *   The HashMap owns its own internal copies independently.
 */
class Authentication {
public:
    // Scan all users to find one whose email matches, then verify the hash.
    // Returns a heap-allocated copy of the User on success, nullptr on failure.
    static User* authenticate(HashMap<int, User>& users,
                              const std::string& email,
                              const std::string& passwordHash);

    // Validate uniqueness, assign a new userId, insert into the map.
    // Returns a heap-allocated copy of the created User, nullptr on duplicate email.
    static User* createUser(HashMap<int, User>& users,
                            const std::string& email,
                            const std::string& passwordHash,
                            const std::string& category);

    // Direct O(1) lookup by userId.
    // Returns a heap-allocated copy, or nullptr if userId is not found.
    static User* getUserById(HashMap<int, User>& users, int userId);

    // Returns the category string for a userId, or "" if not found.
    static std::string getUserCategory(HashMap<int, User>& users, int userId);

    // Updates the score field of an existing user in the map.
    // Returns true on success, false if userId is not found.
    static bool updateUserScore(HashMap<int, User>& users, int userId, float newScore);

private:
    // Auto-incrementing ID counter; starts at 1, never reuses a value.
    // In production this would be seeded from the database on startup.
    static int nextUserId_;

    // Scans every bucket chain looking for a user with a matching email.
    // Returns a raw pointer into the bucket node (not a copy) — internal only.
    static User* findByEmail(HashMap<int, User>& users, const std::string& email);
};

#endif // Authentication_H
