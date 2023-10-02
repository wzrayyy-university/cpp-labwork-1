#include <limits>
#include <filesystem>

const char kDefaultDelimiter = '\n';
const uint64_t kDefaultLinesValue = std::numeric_limits<uint64_t>::max();

struct Arguments {
  static constexpr char kLines[] = "--lines";
  static constexpr char kLinesShort[] = "-l";
  static constexpr char kDelimiter[] = "--delimiter";
  static constexpr char kDelimiterShort[] = "-d";
  static constexpr char kTail[] = "--tail";
  static constexpr char kTailShort[] = "-t";
};

struct ArgumentsWithValue {
  static constexpr char kLines[] = "--lines=";
  static constexpr char kLinesShort[] = "-l=";
  static constexpr char kDelimiter[] = "--delimiter=";
  static constexpr char kDelimiterShort[] = "-d=";
};

struct Config {
  uint64_t lines = kDefaultLinesValue;
  bool tail = false;
  char delimiter = kDefaultDelimiter;
  char* filename{};
};


class ArgumentParser {
 private:
  Config& config;
  int argc;
  char** argv;
  bool is_lines_set{};
  bool is_filename_set{};

  char GetSpecialChar(char ch[]);

  void GetLines(char arg[], int& idx);

  void GetDelimiter(char arg[], int& i);

 public:
  ArgumentParser(int argc, char** argv, Config& config);

  void ParseArguments();
};