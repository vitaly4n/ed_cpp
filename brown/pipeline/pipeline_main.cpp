#include "pipeline.cpp"

#include "test_runner.h"
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

void
TestSanity()
{
  string input = ("erich@example.com\n"
                  "richard@example.com\n"
                  "Hello there\n"
    
                  "erich@example.com\n"
                  "ralph@example.com\n"
                  "Are you sure you pressed the right button?\n"

                  "ralph@example.com\n"
                  "erich@example.com\n"
                  "I do not make mistakes of that kind\n");
  istringstream inStream(input);
  ostringstream outStream;

  PipelineBuilder builder(inStream);
  builder.FilterBy(
    [](const Email& email) { return email.from == "erich@example.com"; });
  builder.CopyTo("richard@example.com");
  builder.Send(outStream);
  auto pipeline = builder.Build();

  pipeline->Run();

  string expectedOutput = ("erich@example.com\n"
                           "richard@example.com\n"
                           "Hello there\n"

                           "erich@example.com\n"
                           "ralph@example.com\n"
                           "Are you sure you pressed the right button?\n"

                           "erich@example.com\n"
                           "richard@example.com\n"
                           "Are you sure you pressed the right button?\n");

  ASSERT_EQUAL(expectedOutput, outStream.str());
}

int
main()
{
  TestRunner tr;
  RUN_TEST(tr, TestSanity);

  int i;
  cin >> i;

  return 0;
}
