#include <cstdlib>
#include <string>

using namespace std;

struct Address
{
  string city, street;
  int building;

  bool operator==(const Address& other) const 
  {
    return city == other.city && street == other.street &&
           building == other.building;
  }
};

struct Person
{
  string name;
  int height;
  double weight;
  Address address;

  bool operator==(const Person& other) const 
  {
    return name == other.name && height == other.height &&
           weight == other.weight && address == other.address;
  }
};

struct AddressHasher
{
  size_t operator()(const Address& obj) const
  {
    const auto r1 = hash_string_(obj.city);
    const auto r2 = hash_string_(obj.street);
    const auto r3 = hash_int_(obj.building);
    const auto x = size_t{ 47 };

    return r1 * x * x + r2 * x + r3;
  }

  hash<string> hash_string_;
  hash<int> hash_int_;
};

struct PersonHasher
{
  size_t operator()(const Person& obj) const
  {
    const auto r1 = hash_address_(obj.address);
    const auto r2 = hash_string_(obj.name);
    const auto r3 = hash_int_(obj.height);
    const auto r4 = hash_double_(obj.weight);
    const auto x = size_t{ 47 };

    return r1 * x * x * x + r2 * x * x + r3 * x + r4;
  }

  AddressHasher hash_address_;
  hash<string> hash_string_;
  hash<int> hash_int_;
  hash<double> hash_double_;
};
