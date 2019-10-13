#include "test_runner.h"

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

#ifdef LOCAL_TEST

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

class Domain
{
public:
  explicit Domain(string_view text)
  {
    vector<string_view> parts = Split(text, ".");
    parts_reversed_.assign(rbegin(parts), rend(parts));
  }

  size_t GetPartCount() const { return parts_reversed_.size(); }

  auto GetParts() const
  {
    return Range(rbegin(parts_reversed_), rend(parts_reversed_));
  }
  auto GetReversedParts() const
  {
    return Range(begin(parts_reversed_), end(parts_reversed_));
  }

  bool operator==(const Domain& other) const
  {
    return parts_reversed_ == other.parts_reversed_;
  }

private:
  vector<string> parts_reversed_;
};

ostream&
operator<<(ostream& stream, const Domain& domain)
{
  bool first = true;
  for (const string_view part : domain.GetParts()) {
    if (!first) {
      stream << '.';
    } else {
      first = false;
    }
    stream << part;
  }
  return stream;
}

// domain is subdomain of itself
bool
IsSubdomain(const Domain& subdomain, const Domain& domain)
{
  const auto subdomain_reversed_parts = subdomain.GetReversedParts();
  const auto domain_reversed_parts = domain.GetReversedParts();
  return subdomain.GetPartCount() >= domain.GetPartCount() &&
         equal(begin(domain_reversed_parts),
               end(domain_reversed_parts),
               begin(subdomain_reversed_parts));
}

bool
IsSubOrSuperDomain(const Domain& lhs, const Domain& rhs)
{
  return lhs.GetPartCount() >= rhs.GetPartCount() ? IsSubdomain(lhs, rhs)
                                                  : IsSubdomain(rhs, lhs);
}

class DomainChecker
{
public:
  template<typename InputIt>
  DomainChecker(InputIt domains_begin, InputIt domains_end)
  {
    sorted_domains_.reserve(distance(domains_begin, domains_end));
    for (const Domain& domain : Range(domains_begin, domains_end)) {
      sorted_domains_.push_back(&domain);
    }
    sort(begin(sorted_domains_), end(sorted_domains_), IsDomainLess);
    sorted_domains_ = AbsorbSubdomains(move(sorted_domains_));
  }

  // Check if candidate is subdomain of some domain
  bool IsSubdomain(const Domain& candidate) const
  {
    const auto it = upper_bound(
      begin(sorted_domains_), end(sorted_domains_), &candidate, IsDomainLess);
    if (it == begin(sorted_domains_)) {
      return false;
    }
    return ::IsSubdomain(candidate, **prev(it));
  }

private:
  vector<const Domain*> sorted_domains_;

  static bool IsDomainLess(const Domain* lhs, const Domain* rhs)
  {
    const auto lhs_reversed_parts = lhs->GetReversedParts();
    const auto rhs_reversed_parts = rhs->GetReversedParts();
    return lexicographical_compare(begin(lhs_reversed_parts),
                                   end(lhs_reversed_parts),
                                   begin(rhs_reversed_parts),
                                   end(rhs_reversed_parts));
  }

  static vector<const Domain*> AbsorbSubdomains(vector<const Domain*> domains)
  {
    domains.erase(unique(begin(domains),
                         end(domains),
                         [](const Domain* lhs, const Domain* rhs) {
                           return IsSubOrSuperDomain(*lhs, *rhs);
                         }),
                  end(domains));
    return domains;
  }
};

vector<Domain>
ReadDomains(istream& in_stream = cin)
{
  vector<Domain> domains;

  size_t count;
  in_stream >> count;
  domains.reserve(count);

  for (size_t i = 0; i < count; ++i) {
    string domain_text;
    in_stream >> domain_text;
    domains.emplace_back(domain_text);
  }
  return domains;
}

vector<bool>
CheckDomains(const vector<Domain>& banned_domains,
             const vector<Domain>& domains_to_check)
{
  const DomainChecker checker(begin(banned_domains), end(banned_domains));

  vector<bool> check_results;
  check_results.reserve(domains_to_check.size());
  for (const Domain& domain_to_check : domains_to_check) {
    check_results.push_back(!checker.IsSubdomain(domain_to_check));
  }

  return check_results;
}

void
PrintCheckResults(const vector<bool>& check_results, ostream& out_stream = cout)
{
  for (const bool check_result : check_results) {
    out_stream << (check_result ? "Good" : "Bad") << "\n";
  }
}

#endif // LOCAL_TEST

void
test_read_domains()
{
  vector<string> str_domains;
  str_domains.push_back("ya.ru");
  str_domains.push_back("hello.ya.ru");
  str_domains.push_back("ya.ya.ya");
  str_domains.push_back("com.com");

  ostringstream os;
  os << str_domains.size() << "\n";
  for (const auto& str_domain : str_domains) {
    os << str_domain << "\n";
  }

  istringstream is(os.str());
  const vector<Domain> domains = ReadDomains(is);

  vector<Domain> ref_domains;
  for (const auto& str_domain : str_domains) {
    ref_domains.emplace_back(str_domain);
  }

  ASSERT_EQUAL(domains, ref_domains);
}

void
test_banned_domains()
{
  vector<string> banned_domains_str = {
    "ya.ru", "hello.ya.ru", "com", "vk.com"
  };
  vector<string> check_domains_str = { "new.ya.ru", "hello.ya.ru",  "com.ru",
                                       "com.com",   "video.vk.com", "ru" };

  vector<Domain> banned_domains(begin(banned_domains_str),
                                end(banned_domains_str));
  vector<Domain> check_domains(begin(check_domains_str),
                               end(check_domains_str));

  vector<bool> res = CheckDomains(banned_domains, check_domains);
  vector<bool> ref = { false, false, true, false, false, true };

  ASSERT_EQUAL(res, ref);
}

void
test_good_bad_mixed_up()
{
  vector<bool> data = { true, false };
  ostringstream res;
  PrintCheckResults(data, res);
  
  string ref{ "Good\nBad\n" };

  ASSERT_EQUAL(res.str(), ref.c_str());
}

void
test_subdomain_res()
{
  const vector<string> banned_domains_str = { "ya.ru" };
  const vector<string> check_domains_str = { "ya.ya.ru" };

  const vector<Domain> banned_domains = { begin(banned_domains_str),
                                          end(banned_domains_str) };
  const vector<Domain> check_domains = { begin(check_domains_str),
                                         end(check_domains_str) };

  const vector<bool> res = CheckDomains(banned_domains, check_domains);
  const vector<bool> ref = { false };

  ASSERT_EQUAL(res, ref);
}

void
test_subdomain_inclusion()
{
  const vector<string> banned_domains_str_1 = { "ya.ru", "com.ya.ru" };
  const vector<Domain> banned_domains_1 = { begin(banned_domains_str_1),
                                            end(banned_domains_str_1) };

  const vector<string> check_domains_str = { "com.ya.ru", "com.ya.ya.ru" };
  const vector<Domain> check_domains = { begin(check_domains_str),
                                         end(check_domains_str) };

  DomainChecker checker{ begin(banned_domains_1), end(banned_domains_1) };
  ASSERT_EQUAL(checker.IsSubdomain(check_domains[0]), true);
  ASSERT_EQUAL(checker.IsSubdomain(check_domains[1]), true);
}

void
test_is_subdomain_mix()
{
  string first{ "ya.ru" };
  string second{ "com.ya.ru" };
  Domain domain{ first };
  Domain sub_domain{ second };

  ASSERT_EQUAL(
    IsSubdomain(sub_domain, domain) && !IsSubdomain(domain, sub_domain), true);
}

void
test_self_subdomain()
{
  string domain_str{ "vk.com" };
  Domain domain{ domain_str };

  ASSERT_EQUAL(IsSubdomain(domain, domain), true);
}

void
test_subdomain_reverse()
{
  string domain_str{ "video.vk.com" };
  Domain domain{ domain_str };

  auto res = domain.GetReversedParts();
  vector<string> ref = { "com", "vk", "video" };

  ASSERT_EQUAL(equal(begin(res), end(res), begin(ref), end(ref)), true);
}

void
test_split()
{
  string domain_str{ "video.vk.com" };
  auto res = Split(domain_str, ".");
  vector<string> ref = { "video", "vk", "com" };

  ASSERT_EQUAL(equal(begin(res), end(res), begin(ref), end(ref)), true);
}

void
TestSimple()
{
  test_banned_domains();
  test_read_domains();
  test_good_bad_mixed_up();
  test_subdomain_res();
  test_subdomain_inclusion();
  test_is_subdomain_mix();
  test_self_subdomain();
  test_subdomain_reverse();
  test_split();
}

int
main()
{
  TestRunner tr;
  RUN_TEST(tr, TestSimple);

  return 0;
}
