#pragma once

#include <fstream>
#include <iterator>
#include <string>

#ifdef LOCAL_TEST

#define STRINGIFY(m) #m
#define STRINGIFY2(m) STRINGIFY(m)

using namespace std::string_literals;

std::string
ReadWholeInput(std::istream& in)
{
  return { std::istreambuf_iterator<char>(in), {} };
}

class TestDataHandler
{
public:
  TestDataHandler(const char* test_dir,
                  const char* input_file,
                  const char* ref_file)
    : in(std::string(test_dir) + "/"s + input_file)
    , ref(std::string(test_dir) + "/"s + ref_file)
  {}

  std::istream& InputData() { return in; }
  std::istream& ReferenceData() { return ref; }

private:
  std::ifstream in;
  std::ifstream ref;
};



#endif
