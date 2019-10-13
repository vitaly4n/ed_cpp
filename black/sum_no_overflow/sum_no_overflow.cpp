#include <iostream>
#include <limits>
#include <optional>

#ifdef LOCAL_TEST
#include "test_runner.h"
#endif

using namespace std;

optional<int64_t>
GetSum(int64_t first, int64_t second)
{
  if ((first >= 0 && numeric_limits<int64_t>::max() - first >= second) ||
      (first < 0 && numeric_limits<int64_t>::min() - first <= second)) {
    return first + second;
  } else {
    return nullopt;
  }
}

#ifdef LOCAL_TEST

void
TestPosPosOverflow()
{
  const int64_t first = numeric_limits<int64_t>::max() - 1;
  const int64_t second = 2;
  ASSERT_EQUAL(GetSum(first, second).has_value(), false);
  ASSERT_EQUAL(GetSum(second, first).has_value(), false);
}

void
TestPosPosGood()
{
  const int64_t first = numeric_limits<int64_t>::max() - 1;
  const int64_t second = 1;
  ASSERT_EQUAL(GetSum(first, second).has_value(), true);
  ASSERT_EQUAL(GetSum(second, first).has_value(), true);
}

void
TestNegNegOverflow()
{
  const int64_t first = numeric_limits<int64_t>::min() + 1;
  const int64_t second = -2;
  ASSERT_EQUAL(GetSum(first, second).has_value(), false);
  ASSERT_EQUAL(GetSum(second, first).has_value(), false);
}

void
TestNegNegGood()
{
  const int64_t first = numeric_limits<int64_t>::min() + 1;
  const int64_t second = +1;
  ASSERT_EQUAL(GetSum(first, second).has_value(), true);
  ASSERT_EQUAL(GetSum(second, first).has_value(), true);
}

void
TestPosNeg()
{
  const int64_t first = numeric_limits<int64_t>::max();
  const int64_t second = numeric_limits<int64_t>::min();
  ASSERT_EQUAL(GetSum(first, second).has_value(), true);
  ASSERT_EQUAL(GetSum(second, first).has_value(), true);
}

void
RunTests()
{
  TestRunner tr;
  RUN_TEST(tr, TestPosPosOverflow);
  RUN_TEST(tr, TestPosPosGood);
  RUN_TEST(tr, TestNegNegOverflow);
  RUN_TEST(tr, TestNegNegGood);
  RUN_TEST(tr, TestPosNeg);
}

#endif

int
main(int, char*[])
{
#ifdef LOCAL_TEST
  RunTests();
#endif
  int64_t first, second;
  cin >> first >> second;

  auto sum_res = GetSum(first, second);
  if (sum_res) {
    cout << *sum_res;
  } else {
    cout << "Overflow!";
  }
}
