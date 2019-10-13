#include <algorithm>
#include <map>
#include <vector>

using namespace std;

template <typename String>
using Group = vector<String>;

template <typename String>
using Char = typename String::value_type;

template <typename String>
struct StringGroupComparator {
  bool operator()(String const& lhs, String const& rhs) const {
    return lexicographical_compare(begin(lhs), end(lhs), begin(rhs), end(rhs));
  }
};

template <typename String>
vector<Group<String>> GroupHeavyStrings(vector<String> strings) {
  map<String, Group<String>, StringGroupComparator<String>> groups;
  for (auto& string : strings) {
    auto charset = string;
    sort(begin(charset), end(charset));
    charset.erase(unique(begin(charset), end(charset)), end(charset));
    groups[move(charset)].push_back(move(string));
  }

  vector<Group<String>> res;
  for (auto& group : groups) {
    res.push_back(Group<String>{make_move_iterator(begin(group.second)),
                                make_move_iterator(end(group.second))});
  }

  return res;
}