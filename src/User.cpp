#include "User.h"

#include <sstream>
#include <iostream>
#include <iomanip>
#include "DTOs.h"      // UserInput, UserDTO, AuthResult
#include "HashMap.h"

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

User::User(int userId,
           const std::string& email,
           const std::string& passwordHash,
           const std::string& category,
           float              score,
           time_t             createdAt)
    : userId_(userId)
    , email_(email)
    , passwordHash_(passwordHash)
    , category_(category)
    , score_(score)
    , createdAt_(createdAt == 0 ? std::time(nullptr) : createdAt)
    , views_(0), clicks_(0), cartAdds_(0), purchases_(0)
    , userLevel_("Normal")
{}

User::User()
    : userId_(-1), score_(0.0f), createdAt_(0)
    , views_(0), clicks_(0), cartAdds_(0), purchases_(0)
    , userLevel_("Normal")
{}

// ---------------------------------------------------------------------------
// Getters
// ---------------------------------------------------------------------------

int                User::getUserId()       const { return userId_; }
const std::string& User::getEmail()        const { return email_; }
const std::string& User::getPasswordHash() const { return passwordHash_; }
const std::string& User::getCategory()     const { return category_; }
const std::string& User::getName()   const { return name_; }
const std::string& User::getRegion() const { return region_; }
float              User::getScore()        const { return score_; }
time_t             User::getCreatedAt()    const { return createdAt_; }
int                User::getViews()        const { return views_; }
int                User::getClicks()       const { return clicks_; }
int                User::getCartAdds()     const { return cartAdds_; }
int                User::getPurchases()    const { return purchases_; }
const std::string& User::getUserLevel()    const { return userLevel_; }

// activityScore = views×1 + clicks×2 + cartAdds×5 + purchases×10
int User::getActivityScore() const {
    return views_ * 1 + clicks_ * 2 + cartAdds_ * 5 + purchases_ * 10;
}

// ---------------------------------------------------------------------------
// Setters
// ---------------------------------------------------------------------------

void User::setEmail(const std::string& email)         { email_ = email; }
void User::setPasswordHash(const std::string& hash)   { passwordHash_ = hash; }
void User::setCategory(const std::string& category)   { category_ = category; }
void User::setScore(float score)                       { score_ = score; }
void User::setName(const std::string& n)   { name_ = n; }
void User::setRegion(const std::string& r) { region_ = r; }

// Activity counter increments — each one recomputes the level afterwards
void User::addViews(int n)     { views_     += n; updateLevel(); }
void User::addClicks(int n)    { clicks_    += n; updateLevel(); }
void User::addCartAdds(int n)  { cartAdds_  += n; updateLevel(); }
void User::addPurchases(int n) { purchases_ += n; updateLevel(); }

// ---------------------------------------------------------------------------
// updateLevel
// Derived from activityScore thresholds defined in the original spec:
//   > 1000 → VIP, > 500 → Active, else → Normal
// Called automatically after any activity counter changes.
// ---------------------------------------------------------------------------
void User::updateLevel() {
    int as = getActivityScore();
    if      (as > 1000) userLevel_ = "VIP";
    else if (as > 500)  userLevel_ = "Active";
    else                userLevel_ = "Normal";
}

// ---------------------------------------------------------------------------
// toString
// Returns a single-line summary of the user for debugging / display().
// Does NOT print the password hash for basic safety hygiene.
// ---------------------------------------------------------------------------
std::string User::toString() const {
    // Format createdAt as a readable timestamp
    char timeBuf[32] = "N/A";
    if (createdAt_ != 0)
        std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S",
                      std::localtime(&createdAt_));

    std::ostringstream oss;
    oss << "User{"
        << "id="       << userId_
        << ", email="  << email_
        << ", category=" << category_
        << ", score="  << score_
        << ", level="  << userLevel_
        << ", activityScore=" << getActivityScore()
        << " (v=" << views_ << " cl=" << clicks_
        << " ca=" << cartAdds_ << " pu=" << purchases_ << ")"
        << ", createdAt=" << timeBuf
        << "}";
    return oss.str();
}

// ---------------------------------------------------------------------------
// isValid
// A user is valid when all identity fields are populated.
// score == 0 and a "Normal" level are both legitimate starting states,
// so they are not checked here.
// ---------------------------------------------------------------------------
bool User::isValid() const {
    return userId_ >= 0
        && !email_.empty()
        && !passwordHash_.empty()
        && !category_.empty()
        && createdAt_ != 0;
}


// ===========================================================================
//  Cluster 1 — Auth and Users
//
//  Two file-scoped HashMaps act as the user store:
//    byId_      — HashMap<int, User>          — O(1) lookup by user ID
//    emailToId_ — HashMap<std::string, int>   — O(1) lookup by email
//
//  Both are static so they are invisible outside this translation unit;
//  all access goes through the three free functions below.
//
//  Remember to add these two lines at the bottom of HashMap.cpp:
//    template class HashMap<int,         User>;
//    template class HashMap<std::string, int>;
// ===========================================================================

static HashMap<int, User>          byId_(16, 0.75);
static HashMap<std::string, int>   emailToId_(16, 0.75);
static int                         nextUserId_ = 1;

// ---------------------------------------------------------------------------
// hashPassword
// djb2-style hash, hex-encoded.
// NOTE: replace with bcrypt / argon2 before going to production.
// O(|plain|).
// ---------------------------------------------------------------------------
static std::string hashPassword(const std::string& plain) {
    unsigned long hash = 5381;
    for (unsigned char c : plain)
        hash = ((hash << 5) + hash) ^ c;   // hash * 33 XOR c

    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << hash;
    return oss.str();
}

// ---------------------------------------------------------------------------
// userToDTO
// Converts the internal User object into the public UserDTO that is safe
// to return to the frontend — password hash is never included.
// ---------------------------------------------------------------------------
static UserDTO userToDTO(const User& u) {
    UserDTO dto;
    dto.id             = u.getUserId();
    dto.name           = u.getName();       // add getName() to User if missing
    dto.email          = u.getEmail();
    dto.category       = u.getCategory();
    dto.score          = static_cast<double>(u.getScore());
    dto.region         = u.getRegion();     // add getRegion() to User if missing
    dto.activity_score = u.getActivityScore();
    dto.level          = u.getUserLevel();
    return dto;
}

// ---------------------------------------------------------------------------
// authenticate_user
// 1. Email → ID lookup (O(1)).
// 2. ID → User lookup (O(1)).
// 3. Compare hashed passwords.
// Returns AuthResult with success=false and a generic error on any failure
// (do not reveal whether the email exists — prevents user enumeration).
// ---------------------------------------------------------------------------
AuthResult authenticate_user(const std::string& email,
                             const std::string& password)
{
    // emailToId_.search() returns 0 (default int) when the key is absent.
    // Real IDs start at 1, so 0 is a safe "not found" sentinel.
    int id = emailToId_.search(email);
    if (id == 0)
        return { false, "Invalid email or password", {}, "" };

    User u = byId_.search(id);
    if (!u.isValid())
        return { false, "Invalid email or password", {}, "" };

    if (u.getPasswordHash() != hashPassword(password))
        return { false, "Invalid email or password", {}, "" };

    return { true, "", userToDTO(u), "dev-token-" + std::to_string(id) };
}

// ---------------------------------------------------------------------------
// create_user
// 1. Reject duplicate emails (409 in HTTP terms).
// 2. Assign auto-increment ID.
// 3. Apply spec defaults: score=50, activity_score=0, level="Normal".
// 4. Insert into both maps.
// 5. Return AuthResult with the new user's public DTO and dev token.
// O(1) average (amortised — may trigger HashMap rehash).
// ---------------------------------------------------------------------------
AuthResult create_user(const UserInput& input) {
    // --- duplicate email check ---
    if (emailToId_.search(input.email) != 0)
        return { false, "Email already registered", {}, "" };

    // --- build the User object ---
    int         newId    = nextUserId_++;
    std::string passHash = hashPassword(input.password);

    // score=50 is the spec default; User constructor takes float score.
    User newUser(newId, input.email, passHash, input.category, 50.0f);
    newUser.setName(input.name);     // add setName() to User if missing
    newUser.setRegion(input.region); // add setRegion() to User if missing

    // --- insert into both maps ---
    byId_.insert(newId, newUser);
    emailToId_.insert(input.email, newId);

    return { true, "", userToDTO(newUser), "dev-token-" + std::to_string(newId) };
}

// ---------------------------------------------------------------------------
// get_user_by_id
// Returns the public UserDTO.
// Throws std::out_of_range when the ID does not exist so the Python/FastAPI
// layer can translate it into a 404 response.
// O(1) average.
// ---------------------------------------------------------------------------
UserDTO get_user_by_id(int user_id) {
    User u = byId_.search(user_id);

    // Default-constructed User has userId_ == -1 (see User::User()).
    if (!u.isValid())
        throw std::out_of_range("User not found: id=" + std::to_string(user_id));

    return userToDTO(u);
}