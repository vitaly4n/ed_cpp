#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <list>

using namespace std;

template <typename RandomIt>
void MakeJosephusPermutation(RandomIt first, RandomIt last,
                             uint32_t step_size) {
  list<typename RandomIt::value_type> pool(make_move_iterator(first), make_move_iterator(last));
  
  auto cur_pos = size_t{0};
  auto pool_it = begin(pool);
  while (!pool.empty()) {
    move(pool_it, next(pool_it), first++);
    pool_it = pool.erase(pool_it);
    if (pool.empty()) {
      break;
    }

    auto const new_cur_pos = cur_pos + step_size - 1;
    auto const overflow = int(-(pool.size() - new_cur_pos));
    if (overflow >= 0) {
      pool_it = begin(pool);
      cur_pos = overflow % pool.size();
      advance(pool_it, cur_pos);
    } else {
      advance(pool_it, step_size - 1);
      cur_pos += step_size - 1;
    }
  }
}