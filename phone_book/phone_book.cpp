#include "phone_book.h"

#include "contact.pb.h"

#include <algorithm>

using namespace std;

namespace {

bool
IsPrefix(string_view prefix, string_view str)
{
  if (str.length() < prefix.length()) {
    return false;
  }

  return mismatch(begin(prefix), end(prefix), begin(str)).first == end(prefix);
}

} // namespace

PhoneBook::PhoneBook(vector<Contact> contacts)
  : contacts_(move(contacts))
{
  sort(begin(contacts_), end(contacts_), [](const auto& lhs, const auto& rhs) { return lhs.name < rhs.name; });
}

PhoneBook::ContactRange
PhoneBook::FindByNamePrefix(string_view name_prefix) const
{
  Contact dummy_contact;
  dummy_contact.name = name_prefix; // string copying >_<
  const auto eq_range =
    equal_range(begin(contacts_), end(contacts_), dummy_contact, [&name_prefix](const auto& lhs, const auto& rhs) {
      if (IsPrefix(name_prefix, lhs.name) && IsPrefix(name_prefix, rhs.name)) {
        return false;
      }
      return lhs.name < rhs.name;
    });

  return { eq_range.first, eq_range.second };
}

void
PhoneBook::SaveTo(ostream& output) const
{
  PhoneBookSerialize::ContactList contacts_pb;
  for (const auto& contact : contacts_) {
    PhoneBookSerialize::Contact contact_pb;
    contact_pb.set_name(contact.name);
    for (const auto& phone : contact.phones) {
      contact_pb.add_phone_number(phone);
    }
    if (contact.birthday) {
      PhoneBookSerialize::Date date_pb;
      date_pb.set_day(contact.birthday->day);
      date_pb.set_month(contact.birthday->month);
      date_pb.set_year(contact.birthday->year);
      *contact_pb.mutable_birthday() = date_pb;
    }
    *contacts_pb.add_contact() = move(contact_pb);
  }
  contacts_pb.SerializeToOstream(&output);
}

PhoneBook
DeserializePhoneBook(istream& input)
{
  PhoneBookSerialize::ContactList contacts_pb;
  contacts_pb.ParseFromIstream(&input);
  vector<Contact> contacts;
  contacts.reserve(contacts_pb.contact_size());
  for (const auto& contact_pb : contacts_pb.contact()) {
    Contact contact;
    contact.name = contact_pb.name();
    contact.phones.reserve(contact_pb.phone_number_size());
    for (const auto& phone_number_pb : contact_pb.phone_number()) {
      contact.phones.push_back(phone_number_pb);
    }
    if (contact_pb.has_birthday()) {
      PhoneBookSerialize::Date birthday_pb = contact_pb.birthday();
      contact.birthday = Date{ static_cast<int>(birthday_pb.year()),
                               static_cast<int>(birthday_pb.month()),
                               static_cast<int>(birthday_pb.day()) };
    }
    contacts.push_back(move(contact));
  }
  return PhoneBook(move(contacts));
}
