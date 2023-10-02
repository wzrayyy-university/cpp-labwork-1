#include <iostream>
#include <format>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <limits>

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
  std::filesystem::path filename;
  uint64_t lines = kDefaultLinesValue;
  bool tail = false;
  char delimiter = kDefaultDelimiter;
};

class ArgumentParser {
 private:
  Config& config;
  int argc;
  char** argv;
  bool is_lines_set = false;
  bool is_filename_set = false;

  void PrintHelpMessage(bool is_error) {
    const char options_text[] = "Options:\n"
                                "\t-l, --lines=N\t\tPrints N lines to stdout.\n"
                                "\t-t, --tail\t\tReverse printing direction.\n"
                                "\t-d --delimiter=CHAR\tSet delimiter character to CHAR.\n";

    if (is_error) {
      std::cerr << "\"Usage:\\n\"\n";
      std::cerr << "\t " << argv[0] << R"( [OPTIONS] [FILE]\n\n")";
      std::cerr << options_text;
    } else {
      std::cout << "\"Usage:\\n\"\n";
      std::cout << "\t " << argv[0] << R"( [OPTIONS] [FILE]\n\n")";
      std::cout << options_text;
    }
  };

  char GetSpecialChar(char ch[]) {
    if (strncmp(ch, "\\n", 2) == 0)
      return '\n';
    else if (strncmp(ch, "\\t", 2) == 0)
      return '\t';
    else if (strncmp(ch, "\\\\", 2) == 0)
      return '\\';
    else if (strncmp(ch, "\\\"", 2) == 0)
      return '\"';
    else if (strncmp(ch, "\\\'", 2) == 0)
      return '\'';
    else {
      ErrorMessage(ErrorCodes::WRONG_DELIMITER, ch);
      return '\0';
    }
  };

  void GetLines(char arg[], int& idx) {
    char* temp_lines;
    int lines;
    if (strncmp(arg, ArgumentsWithValue::kLines, strlen(ArgumentsWithValue::kLines)) == 0) {
      temp_lines = arg + strlen(ArgumentsWithValue::kLines);
    } else if (strncmp(arg, ArgumentsWithValue::kLinesShort, strlen(ArgumentsWithValue::kLinesShort)) == 0) {
      temp_lines = arg + strlen(ArgumentsWithValue::kLinesShort);
    } else {
      if (argc > ++idx)
        temp_lines = argv[idx];
      else
        ErrorMessage(ErrorCodes::VALUE_MISSING, arg);
    }
    try {
      lines = std::stoi(temp_lines);
    } catch (std::invalid_argument& e) {
      ErrorMessage(ErrorCodes::WRONG_LINES_COUNT, temp_lines);
    }

    if (lines >= 0)
      config.lines = lines;
    else {
      ErrorMessage(ErrorCodes::WRONG_LINES_COUNT, lines);
    }
  };

  void GetDelimiter(char arg[], int& i) {
    char* value;
    if (strncmp(arg, ArgumentsWithValue::kDelimiter, strlen(ArgumentsWithValue::kDelimiter)) == 0) {
      value = arg + strlen(ArgumentsWithValue::kDelimiter);
    } else if (strncmp(arg, ArgumentsWithValue::kDelimiterShort, strlen(ArgumentsWithValue::kDelimiterShort)) == 0) {
      value = arg + strlen(ArgumentsWithValue::kDelimiterShort);
    } else {
      if (argc > ++i)
        value = argv[i];
      else
        ErrorMessage(ErrorCodes::VALUE_MISSING, arg);
    }

    if (strncmp(value, "'", strlen("\'")) == 0) {
      if (strlen(value) == strlen(R"('*')")) {
        config.delimiter = value[1];
      } else if (strlen(value) == strlen(R"('\*')")) {
        config.delimiter = GetSpecialChar(value + strlen("\'"));
      } else {
        ErrorMessage(ErrorCodes::WRONG_DELIMITER, value);
      }
    } else if (strlen(value) == strlen("*")) {
      config.delimiter = value[0];
    } else if (strlen(value) == strlen(R"(\*)")) {
      config.delimiter = GetSpecialChar(value);
    } else {
      ErrorMessage(ErrorCodes::WRONG_DELIMITER, value);
    }
  };

 public:
  ArgumentParser(int argc, char** argv, Config& config) :
      config(config), argc(argc), argv(argv) { };

  enum ErrorCodes {
    FILENAME_MISSING,
    FILE_UNAVAILABLE,
    WRONG_LINES_COUNT,
    WRONG_DELIMITER,
    WRONG_ARGUMENT,
    VALUE_MISSING,
    TAIL_WITHOUT_LINES,
  };

  template<typename T>
  void ErrorMessage(ErrorCodes error_code, T argument) {
    switch (error_code) {
      case ErrorCodes::FILENAME_MISSING:std::cerr << "ARGUMENT ERROR: Missing filename" << std::endl;
        break;
      case ErrorCodes::WRONG_LINES_COUNT:std::cerr << "ARGUMENT ERROR: Incorrect lines count " << argument << std::endl;
        break;
      case ErrorCodes::WRONG_DELIMITER:std::cerr << "ARGUMENT ERROR: Incorrect delimiter " << argument << std::endl;
        break;
      case ErrorCodes::FILE_UNAVAILABLE:std::cerr << "ERROR: Unable to open " << argument << std::endl;
        break;
      case ErrorCodes::WRONG_ARGUMENT:std::cerr << "ARGUMENT ERROR: " << argument << " doesn't exist" << std::endl;
        break;
      case ErrorCodes::VALUE_MISSING:std::cerr << "ARGUMENT ERROR: Missing value for " << argument << std::endl;
        break;
      case ErrorCodes::TAIL_WITHOUT_LINES:
        std::cerr << "ARGUMENT ERROR: " << argument << " must be used with --lines flag" << std::endl;
        break;
    }
    PrintHelpMessage(true);
    exit(1);
  };

  void ParseArguments() {
    for (int i = 1; i < argc; ++i) {
      if (strncmp(argv[i], "-", 1) == 0) {
        if (strncmp(argv[i], "-h", 2) == 0 || strncmp(argv[i], "--help", 6) == 0) {
          PrintHelpMessage(false);
          exit(0);
        } else if (strncmp(argv[i], Arguments::kTailShort, 2) == 0
            || strncmp(argv[i], Arguments::kTail, strlen(Arguments::kTail)) == 0) {
          if (!is_lines_set) {
            ErrorMessage(ErrorCodes::TAIL_WITHOUT_LINES, argv[i]);
          }
          config.tail = true;
        } else if (strncmp(argv[i], Arguments::kDelimiterShort, 2) == 0
            || strncmp(argv[i], Arguments::kDelimiter, strlen(Arguments::kDelimiter)) == 0) {
          GetDelimiter(argv[i], i);
        } else if (strncmp(argv[i], Arguments::kLinesShort, 2) == 0
            || strncmp(argv[i], Arguments::kLines, strlen(Arguments::kLines)) == 0) {
          is_lines_set = true;
          GetLines(argv[i], i);
        } else {
          ErrorMessage(ErrorCodes::WRONG_ARGUMENT, argv[i]);
        }
      } else {
        is_filename_set = true;
        config.filename = argv[i];
      }
    }
    if (!is_filename_set) {
      ErrorMessage(ErrorCodes::FILENAME_MISSING, "");
    }
  }
};

class ConsoleReader {
 private:
  static void ReadFileToStdout(std::ifstream& file, const uint64_t& lines, const char& delimiter) {
    int64_t counter = 0;
    const int kBufferSize = 1024;
    char buffer[kBufferSize];
    while (!file.eof()) {
      file.read(buffer, kBufferSize);
      std::streamsize bytes_read = file.gcount();
      for (int i = 0; i < bytes_read; ++i) {
        if (counter >= lines) return;
        if (buffer[i] == delimiter) ++counter;
        std::cout << buffer[i];
      }
    }
  };

  static int64_t FindDelimiter(std::ifstream& file, const uint64_t& lines, const char& delimiter) {
    int64_t counter = 0;

    file.seekg(0, std::ios::end);
    int64_t fpos = file.tellg();

    const int kBufferSize = 1024;
    char buffer[kBufferSize];

    int bytes_to_read = static_cast<int>(std::min(static_cast<int64_t>(kBufferSize), fpos));

    while (bytes_to_read != 0) {
      file.seekg(fpos - bytes_to_read);
      file.read(buffer, bytes_to_read);

      for (int i = bytes_to_read - 1; i >= 0; --i) {
        if (counter >= lines) {
          file.clear();
          return fpos - bytes_to_read + i + 1;
        }
        if (buffer[i] == delimiter)
          ++counter;
      }
      fpos -= bytes_to_read;
      bytes_to_read = static_cast<int>(std::min(static_cast<int64_t>(kBufferSize), fpos));
    }
    file.clear();
    return 0;
  };
 public:
  static void ReadFile(std::ifstream& file, const uint64_t& lines, const bool& tail, const char& delimiter) {
    if (tail) {
      int64_t pos = FindDelimiter(file, lines, delimiter);
      file.seekg(pos);
      ReadFileToStdout(file, kDefaultLinesValue, kDefaultDelimiter);
      return;
    }
    ReadFileToStdout(file, lines, delimiter);
  };
};

int main(int argc, char** argv) {
  Config config;
  ArgumentParser parser(argc, argv, config);

  parser.ParseArguments();

  std::ifstream file(config.filename);

  if (!file.is_open()) {
    parser.ErrorMessage(ArgumentParser::ErrorCodes::FILE_UNAVAILABLE, config.filename);
  }

  ConsoleReader::ReadFile(file, config.lines, config.tail, config.delimiter);

  return 0;
}
