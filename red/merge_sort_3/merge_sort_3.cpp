#include <algorithm>
#include <array>
#include <vector>

using namespace std;

template <typename RandomIt>
void MergeSort(RandomIt range_begin, RandomIt range_end) {
  auto const range_size = distance(range_begin, range_end);
  if (range_size <= 2) {
    return;
  }

  vector<typename RandomIt::value_type> range{make_move_iterator(range_begin),
                                              make_move_iterator(range_end)};

  array<pair<RandomIt, RandomIt>, 3> subranges;
  auto const subrange_size = range_size / 3;
  for (auto i = 0; i < 3; ++i) {
    subranges[i].first = range_begin + subrange_size * i;
    subranges[i].second = range_begin + subrange_size * (i + 1);
    MergeSort<RandomIt>(subranges[i].first, subranges[i].second);
  }

  vector<typename RandomIt::value_type> temp;
  merge(make_move_iterator(subranges[0].first),
        make_move_iterator(subranges[0].second),
        make_move_iterator(subranges[1].first),
        make_move_iterator(subranges[1].second), back_inserter(temp));

  merge(make_move_iterator(begin(temp)), make_move_iterator(end(temp)),
        make_move_iterator(subranges[2].first),
        make_move_iterator(subranges[2].second), range_begin);
}
