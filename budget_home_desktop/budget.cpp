#include "test_runner.h"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>

using namespace std;

template<typename It>
class Range
{
public:
  Range(It begin, It end)
    : begin_(begin)
    , end_(end)
  {}
  It begin() const { return begin_; }
  It end() const { return end_; }

private:
  It begin_;
  It end_;
};

pair<string_view, optional<string_view>>
SplitTwoStrict(string_view s, string_view delimiter = " ")
{
  const size_t pos = s.find(delimiter);
  if (pos == s.npos) {
    return { s, nullopt };
  } else {
    return { s.substr(0, pos), s.substr(pos + delimiter.length()) };
  }
}

vector<string_view>
Split(string_view s, string_view delimiter = " ")
{
  vector<string_view> parts;
  if (s.empty()) {
    return parts;
  }
  while (true) {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
    parts.push_back(lhs);
    if (!rhs_opt) {
      break;
    }
    s = *rhs_opt;
  }
  return parts;
}

class Date
{
public:
  Date() = default;

  Date(unsigned day, unsigned month, unsigned year)
    : day_{ day }
    , month_{ month }
    , year_{ year }
  {}

  unsigned day() const { return day_; }
  unsigned month() const { return month_; }
  unsigned year() const { return year_; }

  unsigned to_timestamp() const
  {
    tm t;
    t.tm_sec = 0;
    t.tm_min = 0;
    t.tm_hour = 0;
    t.tm_mday = day_;
    t.tm_mon = month_ - 1;
    t.tm_year = year_ - 1900;
    t.tm_isdst = 0;
    return mktime(&t);
  }

  unsigned day_number_stamp() const { return to_timestamp() / (60 * 60 * 24); }

private:
  friend istream& operator>>(istream&, Date&);

  unsigned day_ = 0;
  unsigned month_ = 0;
  unsigned year_ = 0;
};

istream&
operator>>(istream& is, Date& date)
{
  string date_formatted;
  is >> date_formatted;

  auto date_data = Split(date_formatted, "-");

  auto set_from_chars = [&date_data](size_t idx, auto& val) {
    from_chars(date_data[idx].data(),
               date_data[idx].data() + date_data[idx].size(),
               val);
  };

  set_from_chars(0, date.year_);
  set_from_chars(1, date.month_);
  set_from_chars(2, date.day_);

  return is;
}

class Budget
{
public:
  void earn(const Date& from, const Date& to, double value);
  void pay_tax(const Date& from, const Date& to);
  double compute_income(const Date& from, const Date& to) const;

private:
  map<unsigned, double> days_budget_;
};

void
Budget::earn(const Date& from, const Date& to, double value)
{
  const auto from_day_num = from.day_number_stamp();
  const auto to_day_num = to.day_number_stamp();

  const auto day_earn = value / (to_day_num - from_day_num + 1.);
  for (auto date = from_day_num; date <= to_day_num; ++date) {
    days_budget_[date] += day_earn;
  }
}

void
Budget::pay_tax(const Date& from, const Date& to)
{
  const auto from_day_num = from.day_number_stamp();
  const auto to_day_num = to.day_number_stamp();

  const auto from_it = days_budget_.lower_bound(from_day_num);
  const auto to_it = days_budget_.upper_bound(to_day_num);
  for (auto& [day, value] : Range(from_it, to_it)) {
    value *= 0.87;
  }
}

double
Budget::compute_income(const Date& from, const Date& to) const
{
  const auto from_day_num = from.day_number_stamp();
  const auto to_day_num = to.day_number_stamp();

  auto res = 0.;
  const auto from_it = days_budget_.lower_bound(from_day_num);
  const auto to_it = days_budget_.upper_bound(to_day_num);
  for (const auto [day, value] : Range(from_it, to_it)) {
    res += value;
  }
  return res;
}

#ifdef LOCAL_TEST

void
test_earn()
{
  Budget budget;
  budget.earn({ 1, 1, 2010 }, { 10, 1, 2010 }, 30);
  budget.earn({ 3, 1, 2010 }, { 31, 1, 2010 }, 50);
  budget.earn({ 10, 1, 2010 }, { 20, 1, 2010 }, 20);
  ASSERT_EQUAL(
    to_string(budget.compute_income({ 1, 1, 2010 }, { 31, 1, 2010 })),
    to_string(100.));
}

void
test_tax()
{
  Budget budget;
  budget.earn({ 1, 1, 2010 }, { 10, 1, 2010 }, 30);
  budget.earn({ 3, 1, 2010 }, { 31, 1, 2010 }, 50);
  budget.earn({ 10, 1, 2010 }, { 20, 1, 2010 }, 20);
  budget.pay_tax({ 1, 1, 2010 }, { 31, 1, 2010 });
  ASSERT_EQUAL(
    to_string(budget.compute_income({ 1, 1, 2010 }, { 31, 1, 2010 })),
    to_string(100 * 0.87));
}

void
test_complete()
{
  Budget budget;
  budget.earn({ 2, 1, 2000 }, { 6, 1, 2000 }, 20);
  auto ref = to_string(budget.compute_income({ 1, 1, 2000 }, { 1, 1, 2001 }));
  ASSERT_EQUAL(ref, to_string(20.));
  budget.pay_tax({ 2, 1, 2000 }, { 3, 1, 2000 });
  ref = to_string(budget.compute_income({ 1, 1, 2000 }, { 1, 1, 2001 }));
  ASSERT_EQUAL(ref, to_string(18.96));
  budget.earn({ 3, 1, 2000 }, { 3, 1, 2000 }, 10);
  ref = to_string(budget.compute_income({ 1, 1, 2000 }, { 1, 1, 2001 }));
  ASSERT_EQUAL(ref, to_string(28.96));
  budget.pay_tax({ 3, 1, 2000 }, { 3, 1, 2000 });
  ref = to_string(budget.compute_income({ 1, 1, 2000 }, { 1, 1, 2001 }));
  ASSERT_EQUAL(ref, to_string(27.2076));
}

void
test_date_input()
{
  istringstream first_jan_2000{ "2000-01-01" };
  istringstream third_feb_2001{ "2001-02-03" };

  Date first;
  Date second;

  first_jan_2000 >> first;
  ASSERT_EQUAL(first.year() == 2000 && first.month() == 1 && first.day() == 1,
               true);

  third_feb_2001 >> second;
  ASSERT_EQUAL(
    second.year() == 2001 && second.month() == 2 && second.day() == 3, true)
}

#endif // LOCAL_TEST

int
main()
{
#ifdef LOCAL_TEST
  TestRunner tr;
  RUN_TEST(tr, test_earn);
  RUN_TEST(tr, test_tax);
  RUN_TEST(tr, test_date_input);
  RUN_TEST(tr, test_complete);

#endif // LOCAL_TEST

  Budget budget;

  unsigned num_queries = 0;
  cin >> num_queries;
  for (unsigned query_idx = 0; query_idx < num_queries; ++query_idx) {
    string query;
    cin >> query;
    if (query == "ComputeIncome") {
      Date from;
      Date to;
      cin >> from >> to;
      cout << budget.compute_income(from, to);
    } else if (query == "Earn") {
      Date from;
      Date to;
      double value;
      cin >> from >> to >> value;
      budget.earn(from, to, value);
    } else if (query == "PayTax") {
      Date from;
      Date to;
      cin >> from >> to;
      budget.pay_tax(from, to);
    }
  }

  return 0;
}
