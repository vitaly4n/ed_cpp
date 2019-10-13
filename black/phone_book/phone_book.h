#pragma once

#include "iterator_range.h"

#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct Date
{
  int year, month, day;
};

struct Contact
{
  std::string name;
  std::optional<Date> birthday;
  std::vector<std::string> phones;
};

class PhoneBook
{
public:
  using ContactRange = IteratorRange<std::vector<Contact>::const_iterator>;

  explicit PhoneBook(std::vector<Contact> contacts);
  ContactRange FindByNamePrefix(std::string_view name_prefix) const;
  void SaveTo(std::ostream& output) const;

private:
  std::vector<Contact> contacts_;
};

PhoneBook
DeserializePhoneBook(std::istream& input);
