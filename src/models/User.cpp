#include "User.h"
#include <sstream>
#include <iostream>

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