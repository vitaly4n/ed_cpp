#include <fstream>
#include <iostream>
#include <string_view>

#include "application.h"

using namespace std;

int
main(int argc, const char* argv[])
{
  if (argc != 2) {
    cerr << "Usage: transport_catalog_part_o [make_base|process_requests]\n";
    return 5;
  }

  const string_view mode(argv[1]);

  if (mode == "make_base") {
    RunBase(cin);
  } else if (mode == "process_requests") {
    RunProcessRequests(cin);
  }

  return 0;
}
