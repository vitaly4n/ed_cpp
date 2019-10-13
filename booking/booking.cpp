#include <deque>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

using namespace std;

static long long QUERY_TIME = 86400;

struct BookInfo {
  BookInfo(long long time, int client_id, int room_count)
      : time_{time}, client_id_{client_id}, room_count_{room_count} {}

  long long time_ = 0;
  int client_id_ = 0;
  int room_count_ = 0;
};

class Booker {
 public:
  void book(long long time, int client_id, int room_count,
            long long last_book) {
    book_info_.emplace_back(time, client_id, room_count);
    clients_info_[client_id] += room_count;
    rooms_booked_ += room_count;

    update_cache(last_book);
  }

  int clients(long long last_book) {
    update_cache(last_book);
    return int(clients_info_.size());
  }
  int rooms(long long last_book) {
    update_cache(last_book);
    return rooms_booked_;
  }

 private:
  void update_cache(long long last_book) {
    if (book_info_.empty()) {
      return;
    }

    while (!book_info_.empty() && last_book - book_info_.front().time_ >= QUERY_TIME) {
      BookInfo& bi = book_info_.front();
      rooms_booked_ -= bi.room_count_;

      auto ciIt = clients_info_.find(bi.client_id_);
      ciIt->second -= bi.room_count_;
      if (ciIt->second <= 0) {
        clients_info_.erase(ciIt);
        ciIt = end(clients_info_);
      }
      book_info_.pop_front();
    }
  }

 private:
  deque<BookInfo> book_info_;
  unordered_map<int, int> clients_info_;
  int rooms_booked_ = 0;
};

class HotelBooker {
 public:
  void book(long long time, string const& hotel_name, int client_id,
            int room_count) {
    last_book_ = time;
    hotel_bookers_[hotel_name].book(time, client_id, room_count, last_book_);
  }
  int clients(string const& hotel_name) {
    return hotel_bookers_[hotel_name].clients(last_book_);
  }
  int rooms(string const& hotel_name) {
    return hotel_bookers_[hotel_name].rooms(last_book_);
  }

 private:
  map<string, Booker> hotel_bookers_;
  long long last_book_ = 0;
};

int main(int argc, char const* argv[]) {
  ios_base::sync_with_stdio(false);
  cin.tie(nullptr);

  auto query_count = 0;
  cin >> query_count;

  HotelBooker hb;

  for (int query_id = 0; query_id < query_count; ++query_id) {
    string query_type;
    cin >> query_type;

    const auto first_char = query_type[0];

    if (first_char == 'B') {
      long long time = 0;
      cin >> time;

      string hotel_name;
      cin >> hotel_name;

      int client_id = 0;
      cin >> client_id;

      int room_count = 0;
      cin >> room_count;

      hb.book(time, hotel_name, client_id, room_count);

    } else if (first_char == 'C') {
      string hotel_name;
      cin >> hotel_name;

      cout << hb.clients(hotel_name) << '\n';

    } else if (first_char == 'R') {
      string hotel_name;
      cin >> hotel_name;

      cout << hb.rooms(hotel_name) << '\n';
    }
  }

  return 0;
}
