#include "ini.h"

#include <utility>

using namespace std;

namespace {

bool
is_section(const string& str)
{
  return !str.empty() && str.front() == '[';
}

bool
is_whiteline(const string& str)
{
  return str.empty() || str.front() == ';';
}

string
get_section_name(string_view v)
{
  auto first = v.find('[');
  auto last = v.find(']');
  return string{ v.substr(first + 1, last - first - 1) };
}

pair<string, string>
get_ini_statement(string_view v)
{
  auto eqsign = v.find('=');
  string key{ v.substr(0, eqsign) };
  string val{ v.substr(eqsign + 1) };
  return { move(key), move(val) };
}

} // namespace

namespace Ini {

Section&
Document::AddSection(string name)
{
  return sections_[move(name)];
}

const Section&
Document::GetSection(const string& name) const
{
  auto it = sections_.find(name);
  if (it == end(sections_)) {
    throw std::out_of_range{"no such section loaded"};
  }
  return it->second;
}

size_t
Document::SectionCount() const
{
  return sections_.size();
}

Document
Load(istream& input)
{
  Document doc;

  string cur_section_name;
  Section* cur_section = nullptr;
  for (string line; getline(input, line);) {
    if (is_section(line)) {
      cur_section_name = get_section_name(line);
      cur_section = &doc.AddSection(cur_section_name);
    }
    else if (!is_whiteline(line)) {
      auto [key, value] = get_ini_statement(line);
      if (!cur_section) {
        cur_section = &doc.AddSection(cur_section_name);
      }
      cur_section->emplace(move(key), move(value));
    }
  }
  return doc;
}

}
