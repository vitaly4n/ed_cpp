#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <vector>

using namespace std;

bool
IsSubdomain(string_view subdomain, string_view domain)
{
  int i = subdomain.size() - 1;
  int j = domain.size() - 1;
  while (i >= 0 && j >= 0) {
    if (subdomain[i--] != domain[j--]) {
      return false;
    }
  }
  return (i < 0 && j < 0) || (j < 0 && subdomain[i] == '.');
}

vector<string>
ReadDomains()
{
  string count_str;
  getline(cin, count_str);
  size_t count = std::stoul(count_str);

  vector<string> domains;
  domains.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    string domain;
    getline(cin, domain);
    domains.push_back(move(domain));
  }
  return domains;
}

struct Compare
{
public:
  bool operator()(string_view lhs, string_view rhs) const
  {
    return lexicographical_compare(
      rbegin(lhs), rend(lhs), rbegin(rhs), rend(rhs));
  }
};

vector<string_view>
DomainBasis(const vector<string>& domains)
{
  vector<string_view> basis_vector{ begin(domains), end(domains) };
  sort(begin(basis_vector),
       end(basis_vector), Compare{});

  auto end_basis_vector =
    unique(begin(basis_vector),
           end(basis_vector),
           [](const auto& lhs, const auto& rhs) {
             return IsSubdomain(lhs, rhs) || IsSubdomain(rhs, lhs);
           });
  basis_vector.erase(end_basis_vector, end(basis_vector));
  return basis_vector;
}

int
main()
{
  const vector<string> banned_domains = ReadDomains();
  const vector<string> domains_to_check = ReadDomains();

  const auto ban_basis = DomainBasis(banned_domains);

  for (const string_view domain : domains_to_check) {
    if (const auto it =
          upper_bound(begin(ban_basis), end(ban_basis), domain, Compare{});
        it != begin(ban_basis) && IsSubdomain(domain, *prev(it))) {
      cout << "Bad" << endl;
    } else {
      cout << "Good" << endl;
    }
  }

  int i;
  cin >> i;

  return 0;
}
