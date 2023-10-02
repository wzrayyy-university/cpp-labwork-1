#include <cstdint>
#include <fstream>

class ConsoleReader {
 private:
  static void ReadFileToStdout(std::ifstream& file, const uint64_t& lines, const char& delimiter);
  static int64_t FindDelimiter(std::ifstream& file, const uint64_t& lines, const char& delimiter);
 public:
  static void ReadFile(std::ifstream& file, const uint64_t& lines, const bool& tail, const char& delimiter);
};