#include <iostream>
#include "airline_ticket.h"

using namespace std;

ostream& operator<<(ostream& os, const Date& date) {
  os << date.day << "-" << date.month << "-" << date.year;
  return os;
}

ostream& operator<<(ostream& os, const Time& time) {
  os << time.hours << ":" << time.minutes;
  return os;
}

bool operator<(const Date& lhs, const Date& rhs) {
  const auto lhs_data = {lhs.year, lhs.month, lhs.day};
  const auto rhs_data = {rhs.year, rhs.month, rhs.day};

  return lexicographical_compare(begin(lhs_data), end(lhs_data),
                                 begin(rhs_data), end(rhs_data));
}

bool operator==(const Date& lhs, const Date& rhs) {
  return lhs.year == rhs.year && lhs.month == rhs.month && lhs.day == rhs.day;
}

bool operator<(const Time& lhs, const Time& rhs) {
  const auto lhs_data = {lhs.hours, lhs.minutes};
  const auto rhs_data = {rhs.hours, rhs.minutes};

  return lexicographical_compare(begin(lhs_data), end(lhs_data),
                                 begin(rhs_data), end(rhs_data));
}

bool operator==(const Time& lhs, const Time& rhs) {
  return lhs.hours == rhs.hours && lhs.minutes == rhs.minutes;
}

#define SORT_BY(field) \
  [](const auto& lhs, const auto& rhs) -> bool { return lhs.field < rhs.field; }
