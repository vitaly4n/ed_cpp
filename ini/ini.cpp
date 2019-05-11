#include "ini.h"

using namespace std;

namespace Ini {

Section&
Document::AddSection(string name)
{
  // TODO
  static Section s;
  return s;
}

const Section&
Document::GetSection(const string& name) const
{
  // TODO
  static Section s;
  return s;
}

size_t
Document::SectionCount() const
{
  // TODO
  return 0;
}

Document
Load(istream& input)
{
  // TODO
  return {};
}

}
