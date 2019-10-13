#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <numeric>

using namespace std;

class ReadingManager {
 public:
  ReadingManager() : user_data_{MAX_USER_COUNT_, end(book_data_)} {}

  void Read(int user_id, int page_count) {
    if (user_id >= user_data_.size()) {
      throw runtime_error("user_id value violates MAX_ID constraint");
    }

    auto emplace_res = book_data_.emplace(page_count, 0);
    emplace_res.first->second += 1;

    auto& book_data_it = user_data_[user_id];
    if (book_data_it != end(book_data_)) {
      if (book_data_it->second <= 1) {
        book_data_.erase(book_data_it); 
      } else {
        book_data_it->second -= 1;
      }
    }
    book_data_it = emplace_res.first;
  }

  double Cheer(int user_id) const {
    if (user_id >= user_data_.size()) {
      throw runtime_error("user_id value violates MAX_ID constraint");
    }

    auto const& book_data_it = user_data_[user_id];
    if (book_data_it == end(book_data_)) {
      return 0.;
    }

    auto sum_op = [](auto const& accumulated, auto const& pair_to_accum) {
      return accumulated + pair_to_accum.second;
    };

    auto const loser_count = accumulate<BookData::const_iterator>(
        begin(book_data_), book_data_it, 0, sum_op);

    auto const competitor_count = accumulate<BookData::const_iterator>(
        book_data_it, end(book_data_), loser_count, sum_op) - 1;

    return competitor_count != 0 ? static_cast<double>(loser_count) / competitor_count : 1.;
  }

 private:
  static const int MAX_USER_COUNT_ = 100'001;

  using BookData = map<int, int>; // map page number - user count
  using UserData = vector<BookData::iterator>;

  BookData book_data_;
  UserData user_data_;
};

int main() {
  ios_base::sync_with_stdio(false);
  cin.tie(nullptr);

  ReadingManager manager;

  int query_count;
  cin >> query_count;

  for (int query_id = 0; query_id < query_count; ++query_id) {
    string query_type;
    cin >> query_type;
    int user_id;
    cin >> user_id;

    if (query_type[0] == 'R') {
      int page_count;
      cin >> page_count;
      manager.Read(user_id, page_count);
    } else if (query_type[0] == 'C') {
      cout << setprecision(6) << manager.Cheer(user_id) << "\n";
    }
  }

  return 0;
}
