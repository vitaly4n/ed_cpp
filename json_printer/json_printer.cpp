#include <test_runner.h>

#include <cassert>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <variant>

////////////////////////////////////////////////////////////////////////////
/// For public use: JsonPrinter
///

class ArrayContext;
class ObjectContext;

////////////////////////////////////////////////////////////////////////////
/// \brief PrintJsonArray returns a proxy array object with various methods
///                       appending specified data to the end of the array.
///                       The array is printed to the output when the object
///                       is destroyed.
/// \param out output stream
/// \return

ArrayContext
PrintJsonArray(std::ostream& out);

////////////////////////////////////////////////////////////////////////////
/// \brief PrintJsonObject returns a proxy dictionary object with Key method
///                        returning an object to which the value of that key
///                        can be assigned. The dictionary is printed to the
///                        output when the object is destroyed.
/// \param out output stream
/// \return
///

ObjectContext
PrintJsonObject(std::ostream& out);

////////////////////////////////////////////////////////////////////////////
/// \brief PrintJsonString prints a quoted string
/// \param out output stream
/// \param str
///

void
PrintJsonString(std::ostream& out, std::string_view str);




////////////////////////////////////////////////////////////////////////////
/// JsonPrinter internal declarations
///

class JsonValue;
using JsonValueArray = std::vector<JsonValue>;
using JsonValueDict = std::vector<std::pair<std::string_view, JsonValue>>;
using JsonNumber = ino64_t;
using JsonString = std::string_view;
using JsonBoolean = bool;
using JsonNull = std::monostate;

template<typename T>
class ObjectPrinter;

template<typename T>
class ArrayPrinter;

template<typename T>
void
PrintJsonValue(std::ostream& out, const T& val);

class JsonValue : std::variant<JsonNull, JsonValueArray, JsonValueDict, JsonNumber, JsonString, JsonBoolean>
{
public:
  using variant::variant;

  variant& AsBase() { return *this; }
  const variant& AsBase() const { return *this; }

  void PrintValue(std::ostream& out) const;
};

class DummyPrinter
{
public:
  DummyPrinter(std::ostream& out);
  ~DummyPrinter() { value_.PrintValue(out_); }

  void SetValue(JsonValue value) { value_ = std::move(value); }

private:
  std::ostream& out_;
  JsonValue value_;
};

template<typename Base>
class ValuePrinter
{
public:
  ValuePrinter(Base& base, std::ostream& out);

  Base& Null();
  Base& Number(JsonNumber number);
  Base& String(JsonString string);
  Base& Boolean(JsonBoolean boolean);
  ObjectPrinter<Base> BeginObject();
  ArrayPrinter<Base> BeginArray();

private:
  Base& base_;
  std::ostream& out_;
};

template<typename Base>
class ArrayPrinter : public ValuePrinter<ArrayPrinter<Base>>
{
public:
  ArrayPrinter(Base& base, std::ostream& out);
  ~ArrayPrinter();

  Base& EndArray();
  void SetValue(JsonValue value);

private:
  Base& base_;
  std::ostream& out_;

  JsonValueArray array_;
  bool terminated_ = false;
};

template<typename Base>
class ObjectPrinter
{
public:
  ObjectPrinter(Base& base, std::ostream& out);
  ~ObjectPrinter();

  ValuePrinter<ObjectPrinter> Key(std::string_view key);
  Base& EndObject();
  void SetValue(JsonValue value);

private:
  Base& base_;
  std::ostream& out_;

  JsonValueDict dict_;
  std::string_view last_key_;
  bool terminated_ = false;
};

class ArrayContext
  : public DummyPrinter
  , public ArrayPrinter<DummyPrinter>
{
public:
  ArrayContext(std::ostream& out)
    : DummyPrinter(out)
    , ArrayPrinter(static_cast<DummyPrinter&>(*this), out)
  {}
};

class ObjectContext
  : public DummyPrinter
  , public ObjectPrinter<DummyPrinter>
{
public:
  ObjectContext(std::ostream& out)
    : DummyPrinter(out)
    , ObjectPrinter(static_cast<DummyPrinter&>(*this), out)
  {}
};

////////////////////////////////////////////////////////////////////////////
/// JsonPrinter internal implementations
///

template<typename T>
void
PrintJsonValue(std::ostream& out, const T& val)
{
  out << val;
}

template<>
void
PrintJsonValue(std::ostream& out, const JsonValueArray& array)
{
  out << "[";
  for (auto it = std::begin(array); it != std::end(array); ++it) {
    if (it != std::begin(array)) {
      out << ",";
    }
    it->PrintValue(out);
  }
  out << "]";
}

template<>
void
PrintJsonValue(std::ostream& out, const JsonValueDict& dict)
{
  out << "{";
  for (auto it = std::begin(dict); it != std::end(dict); ++it) {
    if (it != std::begin(dict)) {
      out << ",";
    }

    out << std::quoted(it->first) << ":";
    it->second.PrintValue(out);
  }
  out << "}";
}

template<>
void
PrintJsonValue(std::ostream& out, const JsonNull&)
{
  out << "null";
}

template<>
void
PrintJsonValue(std::ostream& out, const JsonString& string)
{
  out << quoted(string);
}

template<>
void
PrintJsonValue(std::ostream& out, const JsonBoolean& boolean)
{
  out << (boolean ? "true" : "false");
}

void
JsonValue::PrintValue(ostream& out) const
{
  std::visit([&out](const auto& val) { PrintJsonValue(out, val); }, AsBase());
}

DummyPrinter::DummyPrinter(ostream& out)
  : out_(out)
{}

template<typename Base>
ValuePrinter<Base>::ValuePrinter(Base& base, ostream& out)
  : base_(base)
  , out_(out)
{}

template<typename Base>
Base&
ValuePrinter<Base>::Null()
{
  base_.SetValue({});
  return base_;
}

template<typename Base>
Base&
ValuePrinter<Base>::Number(JsonNumber number)
{
  base_.SetValue(number);
  return base_;
}

template<typename Base>
Base&
ValuePrinter<Base>::String(JsonString string)
{
  base_.SetValue(string);
  return base_;
}

template<typename Base>
Base&
ValuePrinter<Base>::Boolean(JsonBoolean boolean)
{
  base_.SetValue(boolean);
  return base_;
}

template<typename Base>
ArrayPrinter<Base>::ArrayPrinter(Base& base, ostream& out)
  : ValuePrinter<ArrayPrinter<Base>>(*this, out)
  , base_(base)
  , out_(out)
{}

template<typename Base>
ArrayPrinter<Base>::~ArrayPrinter()
{
  if (!terminated_) {
    EndArray();
  }
}

template<typename Base>
Base&
ArrayPrinter<Base>::EndArray()
{
  terminated_ = true;
  base_.SetValue(std::move(array_));
  return base_;
}

template<typename Base>
void
ArrayPrinter<Base>::SetValue(JsonValue value)
{
  array_.push_back(std::move(value));
}

template<typename Base>
ObjectPrinter<Base>::ObjectPrinter(Base& base, ostream& out)
  : base_(base)
  , out_(out)
{}

template<typename Base>
ObjectPrinter<Base>::~ObjectPrinter()
{
  if (!terminated_) {
    EndObject();
  }
}

template<typename Base>
ValuePrinter<ObjectPrinter<Base>>
ObjectPrinter<Base>::Key(std::string_view key)
{
  last_key_ = key;
  return { *this, out_ };
}

template<typename Base>
Base&
ObjectPrinter<Base>::EndObject()
{
  terminated_ = true;
  base_.SetValue(std::move(dict_));
  return base_;
}

template<typename Base>
void
ObjectPrinter<Base>::SetValue(JsonValue value)
{
  dict_.emplace_back(last_key_, std::move(value));
  last_key_ = "";
}

template<typename Base>
ObjectPrinter<Base>
ValuePrinter<Base>::BeginObject()
{
  return { base_, out_ };
}

template<typename Base>
ArrayPrinter<Base>
ValuePrinter<Base>::BeginArray()
{
  return { base_, out_ };
}

ArrayContext
PrintJsonArray(std::ostream& out)
{
  return { out };
}

ObjectContext
PrintJsonObject(std::ostream& out)
{
  return { out };
}

void
PrintJsonString(std::ostream& out, std::string_view str)
{
  out << std::quoted(str);
}

////////////////////////////////////////////////////////////////////////////
/// Tests
///

void
TestArray()
{
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json.Number(5).Number(6).BeginArray().Number(7).EndArray().Number(8).String("bingo!");
  }

  ASSERT_EQUAL(output.str(), R"([5,6,[7],8,"bingo!"])");
}

void
TestObject()
{
  std::ostringstream output;

  {
    auto json = PrintJsonObject(output);
    json.Key("id1").Number(1234).Key("id2").Boolean(false).Key("").Null().Key("\"").String("\\");
  }

  ASSERT_EQUAL(output.str(), R"({"id1":1234,"id2":false,"":null,"\"":"\\"})");
}

void
TestAutoClose()
{
  std::ostringstream output;

  {
    auto json = PrintJsonArray(output);
    json.BeginArray().BeginObject();
  }

  ASSERT_EQUAL(output.str(), R"([[{}]])");
}

int
main()
{
  TestRunner tr;
  RUN_TEST(tr, TestArray);
  RUN_TEST(tr, TestObject);
  RUN_TEST(tr, TestAutoClose);

  return 0;
}
