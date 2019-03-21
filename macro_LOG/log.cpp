#include <iostream>
#include <string>

using namespace std;

class Logger {
 public:
  Logger(ostream& output_stream) : os(output_stream) {}

  void SetLogLine(bool value) { log_line = value; }
  void SetLogFile(bool value) { log_file = value; }

  void Log(const string& message) { os << message << "\n"; }

  void Log(const string& logMsg, char const* fileName, int lineNumber) {
    string composedMsg;
    if (log_file) {
      composedMsg += fileName;
    }
    if (log_line) {
      if (log_file) {
        composedMsg += ":";
      } else {
        composedMsg += "Line ";
      }
      composedMsg += to_string(lineNumber);
    }

    if (!composedMsg.empty()) {
      composedMsg += " ";
    }

    composedMsg += logMsg;
    Log(composedMsg);
  }

 private:
  ostream& os;
  bool log_line = false;
  bool log_file = false;
};

#define LOG(logger, message) logger.Log(message, __FILE__, __LINE__)
