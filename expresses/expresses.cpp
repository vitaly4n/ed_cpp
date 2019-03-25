#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class RouteManager {
 public:
  void add_route(int start, int finish) {
    reachable_sets_[start].insert(finish);
    reachable_sets_[finish].insert(start);
  }

  int ramained_walking_distance(int start, int finish) {
    return min({abs(finish - start), find_nearest_finish(start, finish)});
  }

 private:
  int find_nearest_finish(int start, int finish) {
    auto from_start_it = reachable_sets_.find(start);
    if (from_start_it == end(reachable_sets_)) {
      return abs(start - finish);
    }

    auto const& start_expresses = from_start_it->second;
    auto ge_it = start_expresses.lower_bound(finish);
    if (ge_it == end(start_expresses)) {
      return abs(finish - *prev(ge_it));
    }
    if (*ge_it == finish || ge_it == begin(start_expresses)) {
      return abs(finish - *ge_it);
    }

    return min({abs(*prev(ge_it) - finish), abs(*ge_it - finish)});
  }

 private:
  unordered_map<int, set<int>> reachable_sets_;
};

int main() {
  ios_base::sync_with_stdio(false);
  cin.tie(nullptr);

  RouteManager routes;

  int query_count;
  cin >> query_count;

  for (int query_id = 0; query_id < query_count; ++query_id) {
    string query_type;
    cin >> query_type;
    int start, finish;
    cin >> start >> finish;
    if (query_type == "ADD") {
      routes.add_route(start, finish);
    } else if (query_type == "GO") {
      cout << routes.ramained_walking_distance(start, finish) << "\n";
    }
  }

  return 0;
}
