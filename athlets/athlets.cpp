#include <list>
#include <vector>
#include <iostream>

using namespace std;

class Rank {
 public:
  Rank() : players_{100000, end(rank_)} {}

  void go(int player_num, int player_behind_num) {
    players_[player_num] =
        rank_.insert(players_[player_behind_num], player_num);
  }

  vector<int> get_rank() const { return {cbegin(rank_), cend(rank_)}; }

 private:
  using RankType = list<int>;
  
  RankType rank_;
  vector<RankType::const_iterator> players_;
};

int main(int argc, char* argv[]) { 
  ios_base::sync_with_stdio(false);
  cin.tie(nullptr);

  Rank r;

  int query_num = 0;
  cin >> query_num;
  for (; query_num != 0; --query_num) {
    int player_num = 0, player_behind_num = 0;
    cin >> player_num >> player_behind_num;
    r.go(player_num, player_behind_num);
  }
  for (auto const& player : r.get_rank()) {
    cout << player << " ";
  }
  return 0;
}