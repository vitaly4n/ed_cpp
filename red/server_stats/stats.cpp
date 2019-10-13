#include "stats.h"

#include <algorithm>
#include <vector>

Stats::Stats() 
  : method_stat_{
          {"GET", 0},
          {"POST", 0},
          {"PUT", 0},
          {"DELETE", 0}, 
          {"UNKNOWN", 0}
      },
    uri_stat_{
          {"/", 0},
          {"/order", 0},
          {"/product", 0},
          {"/basket", 0},
          {"/help", 0},
          {"unknown", 0}}
{}

void Stats::AddMethod(string_view method) {
  auto it = method_stat_.find(method);
  if (it != end(method_stat_)) {
    ++it->second;
  } else {
    ++method_stat_["UNKNOWN"];
  }
}

void Stats::AddUri(string_view uri) { 
  auto it = uri_stat_.find(uri);
  if (it != end(uri_stat_)) {
    ++it->second;
  } else {
    ++uri_stat_["unknown"];
  }
}

map<string_view, int> const& Stats::GetMethodStats() const {
  return method_stat_;
}

map<string_view, int> const& Stats::GetUriStats() const { return uri_stat_; }

HttpRequest ParseRequest(string_view line) {
  vector<string_view> req_data;

  while (!line.empty()) {
    auto pos = line.find_first_not_of(' ');
    line.remove_prefix(pos);

    pos = line.find(' ');
    req_data.push_back(line.substr(0, pos));

    line.remove_prefix(min({pos, line.size()}));
  }

  if (req_data.size() != 3) {
    throw std::invalid_argument("request is invalid!");
  }

  return {req_data[0], req_data[1], req_data[2]};
}