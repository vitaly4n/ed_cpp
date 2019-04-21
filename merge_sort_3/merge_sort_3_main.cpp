#include "merge_sort_3.cpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

#include "../test_runner.h"

using namespace std;

void TestIntVector() {
  vector<int> numbers = {6, 1, 3, 9, 1, 9, 8, 12, 1};
  MergeSort(begin(numbers), end(numbers));
  ASSERT(is_sorted(begin(numbers), end(numbers)));
}

int main() {
  TestRunner tr;
  RUN_TEST(tr, TestIntVector);
  int i;
  cin >> i;
  return 0;
}
