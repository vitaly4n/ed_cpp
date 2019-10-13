#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

#include "../profile.h"
#include "../test_runner.h"

using namespace std;

#include "learner.cpp"

void measure_learner() {
  Learner learner;
  vector<int> vals(30000, 0);
  iota(begin(vals), end(vals), 1);
  {
    LOG_DURATION("learn");
    for (const auto& try_number : {1, 2, 3, 4, 5}) {
      for (auto const& val : vals) {
        learner.Learn({to_string(val)});
      }
    }
  }

  {
    LOG_DURATION("report");
    auto const learned = learner.KnownWords();
  }
}
int main() {
  measure_learner();
  return 0;
}
