#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <numeric>

using namespace std;

class BookStatistics {
    static const size_t max_users = 100'000u + 1;
    static const size_t max_pages = 1'000u + 1;

public:
    void SetUserProgress(const unsigned int user, const unsigned int page);
    float GetUserCheer(const unsigned int user) const;

private:
    /* number of pages read by user (user id = index) */
    vector<unsigned int> user_pages_ = vector<unsigned int>(max_users);

    /* number of users read number of pages equal to index */
    vector<unsigned int> user_count_ = vector<unsigned int>(max_pages);
};

void BookStatistics::SetUserProgress(const unsigned int user, const unsigned int page) {
    unsigned int previous_pages = user_pages_[user];
    if (previous_pages != 0) {
        --user_count_[previous_pages];
    }
    user_pages_[user] = page;
    if (page != 0) {
        ++user_count_[page];
    }
}

float BookStatistics::GetUserCheer(const unsigned int user) const {

    unsigned int pages = user_pages_[user];
    if (pages == 0) return 0.;
    int better = reduce(user_count_.begin() + pages, user_count_.end(), -1);
    if (better == 0) return 1.;
    int worse = reduce(user_count_.begin() + 1, user_count_.begin() + pages, 0);
    return static_cast<float>(worse) / static_cast<float>(worse + better);
}

void ProcessRequests(istream& in_stream = cin, ostream& out_stream = cout) {
    BookStatistics stat{};
    unsigned int request_qty;
    in_stream >> request_qty;
    for (unsigned int i = 0; i < request_qty; ++i) {
        string s;
        in_stream >> s;
        if (s == "READ") {
            unsigned int user;
            unsigned int page;
            in_stream >> user >> page;
            stat.SetUserProgress(user, page);
        } else if (s == "CHEER") {
            unsigned int user;
            in_stream >> user;
            out_stream << setprecision(6);
            out_stream << stat.GetUserCheer(user) << endl;
        }
    }
}

int main() {
    ProcessRequests();
}