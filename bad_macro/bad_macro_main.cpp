#include "../test_runner.h"

#include <ostream>
using namespace std;

#include "bad_macro.cpp"

int main() {
  TestRunner tr;
  tr.RunTest(
      [] {
        ostringstream output;
        PRINT_VALUES(output, 5, "red belt");
        ASSERT_EQUAL(output.str(), "5\nred belt\n");
      },
      "PRINT_VALUES usage example");
}
