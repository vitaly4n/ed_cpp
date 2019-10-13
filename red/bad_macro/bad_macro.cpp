
#define PRINT_VALUES(out, x, y)                \
  [&out](const auto& xVal, const auto& yVal) { \
    out << xVal << endl << yVal << endl;       \
  }((x), (y))
