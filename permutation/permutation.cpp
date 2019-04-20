#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <vector>

using namespace std;

template <typename RandomIt>
void MakeJosephusPermutation(RandomIt first, RandomIt last,
                             uint32_t step_size) {
  vector<typename RandomIt::value_type> pool(make_move_iterator(first), make_move_iterator(last));
  
  auto const init_first = first;
  size_t cur_pos = 0;
  while (!pool.empty()) {
    auto pool_it = begin(pool) + cur_pos;
    move(pool_it, next(pool_it), first++);
    pool.erase(pool_it);
    if (pool.empty()) {
      break;
    }
    cur_pos = (cur_pos + step_size - 1) % pool.size();
  }
}