#include "airline_ticket.h"

#include <iostream>

using namespace std;

ostream& operator<<(ostream& os, const Date& date) {
  os << date.year << "-" << date.month << "-" << date.day;
  return os;
}

ostream& operator<<(ostream& os, const Time& time) {
  os << time.hours << ":" << time.minutes;
  return os;
}

istream& operator>>(istream& is, Date& date) { 
  is >> date.year;
  is.ignore(1, '-');
  is >> date.month;
  is.ignore(1, '-');
  is >> date.day;
  
  return is; 
}

istream& operator>>(istream& is, Time& time) { 
  is >> time.hours;
  is.ignore(1, ':');
  is >> time.minutes;
  
  return is;
}

bool operator==(const Date& lhs, const Date& rhs) {
  return lhs.year == rhs.year && lhs.month == rhs.month && lhs.day == rhs.day;
}

bool operator==(const Time& lhs, const Time& rhs) {
  return lhs.hours == rhs.hours && lhs.minutes == rhs.minutes;
}

#define UPDATE_FIELD(ticket, field, values) \
  {                                         \
    auto it = values.find(#field);          \
    if (it != end(values)) {                \
      istringstream is(it->second);         \
      is >> ticket.field;                   \
    }                                       \
  }
