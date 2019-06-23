#include "test_runner.h"

#include "request_plain.h"
#include "transport_manager.h"
#include "utils.h"

#ifdef LOCAL_TEST
#include "tests.h"
#endif

#include <algorithm>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

int
main()
{
#ifdef LOCAL_TEST
  run_tests();
#endif // LOCAL_TEST

  cout << setprecision(6);

  const auto requests = read_requests();
  const auto responses = process_requests(requests);
  print_responses(responses);

  return 0;
}
