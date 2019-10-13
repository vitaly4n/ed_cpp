#include <algorithm>
#include <set>
#include <string>
#include <vector>

using namespace std;

class Learner {
 private:
  set<string> dict;

 public:
  int Learn(const vector<string>& words) {
    auto count = 0;
    for (const auto& word : words) {
      if (dict.emplace(word).second) {
        ++count;
      }
    }
    return count;
  }

  vector<string> KnownWords() {
    vector<string> res{dict.begin(), dict.end()};
    return res;
  }
};
