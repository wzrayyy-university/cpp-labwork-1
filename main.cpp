#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <cstdint>
#include <format>
#include <cstring>
#include <fstream>
#include <limits>

const char kDefaultDelimiter = '\n';

struct Config {
  std::string filename;
  uint64_t lines = -1;
  bool tail = false;
  char delimiter = kDefaultDelimiter;
};

class ArgumentParser {
 private:
  Config &config;
  int argc;
  char **argv;
  bool is_lines_set;

  void PrintHelpMessage(bool is_error) {
    std::string message = std::format("Usage:\n"
                                      "\t {} [OPTIONS] [FILE]\n\n"
                                      "Options:\n"
                                      "\t-l, --lines=N\t\tPrints N lines to stdout.\n"
                                      "\t-t, --tail\t\tReverse printing direction.\n"
                                      "\t-d --delimiter=CHAR\tSet delimiter character to CHAR.\n", argv[0]);
    if (is_error)
      std::cerr << message;
    else
      std::cout << message;
  };

  char GetSpecialChar(const std::string &ch) {
    if (ch == "\\n")
      return '\n';
    else if (ch == "\\t")
      return '\t';
    else if (ch == "\\\\")
      return '\\';
    else if (ch == "\\\"")
      return '\"';
    else if (ch == "\\'")
      return '\'';
    else {
      ErrorMessage(ErrorCodes::WRONG_DELIMITER, ch);
      return '\0';
    }
  };

  void GetLines(const std::string &arg, int &i) {
    std::string temp_lines;
    int lines;
    if (arg.starts_with("--lines=")) {
      temp_lines = arg.substr(strlen("--lines="), arg.size() - strlen("--lines="));
    } else if (arg.starts_with("-l=")) {
      temp_lines = arg.substr(strlen("-l="), arg.size() - strlen("-l="));
    } else {
      if (argc > ++i)
        temp_lines = argv[i];
      else
        ErrorMessage(ErrorCodes::VALUE_MISSING, arg);
    }
    try {
      lines = std::stoi(temp_lines);
    } catch (std::invalid_argument &e) {
      ErrorMessage(ErrorCodes::WRONG_LINES_COUNT, temp_lines);
    }

    if (lines >= 0)
      config.lines = lines;
    else {
      ErrorMessage(ErrorCodes::WRONG_LINES_COUNT, lines);
    }
  };

  void GetDelimiter(const std::string &arg, int &i) {
    std::string value;
    if (arg.starts_with("--delimiter=")) {
      size_t prefix_len = strlen("--delimiter=");
      value = arg.substr(prefix_len, arg.size() - prefix_len);
    } else if (arg.starts_with("-d=")) {
      size_t prefix_len = strlen("-d=");
      value = arg.substr(prefix_len, arg.size() - prefix_len);
    } else {
      if (argc > ++i)
        value = argv[i];
      else
        ErrorMessage(ErrorCodes::VALUE_MISSING, arg);
    }

    if (value.starts_with("'")) {
      if (value.size() == 3) {
        config.delimiter = value[1];
      } else if (value.size() == 4) {
        config.delimiter = GetSpecialChar(value.substr(1, value.size() - 2));
      } else {
        ErrorMessage(ErrorCodes::WRONG_DELIMITER, value);
      }
    } else if (value.size() == 1) {
      config.delimiter = value[0];
    } else if (value.size() == 2) {
      config.delimiter = GetSpecialChar(value);
    } else {
      ErrorMessage(ErrorCodes::WRONG_DELIMITER, value);
    }
  };

 public:
  ArgumentParser(int argc, char **argv, Config &config) :
      config(config), argc(argc), argv(argv) {};

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
      std::string arg = argv[i];
      if (arg.starts_with("-")) {
        if (arg == "-h" || arg == "--help") {
          PrintHelpMessage(false);
          exit(0);
        } else if (arg.contains("-t")) {
          if (!is_lines_set) {
            ErrorMessage(ErrorCodes::TAIL_WITHOUT_LINES, arg);
          }
          config.tail = true;
        } else if (arg.contains("-d")) {
          GetDelimiter(arg, i);
        } else if (arg.contains("-l")) {
          is_lines_set = true;
          GetLines(arg, i);
        } else {
          ErrorMessage(ErrorCodes::WRONG_ARGUMENT, arg);
        }
      } else {
        config.filename = arg;
      }
    }
    if (config.filename.empty()) {
      ErrorMessage(ErrorCodes::FILENAME_MISSING, "");
    }
  }
};

class FileReader {
 private:
  std::string filename;
  Config config;
  std::ifstream file;
 public:
  FileReader(std::string filename, Config &config) : filename(std::move(filename)), config(config) {
    file.open(this->filename);
  };

  bool IsOpen() {
    return file.is_open();
  }

  void ReadFile() { // простите...
    long long counter = 0;
    constexpr long long buffer_size = 1024*128;
    char buffer[buffer_size];

    if (!config.tail) {
      while (!file.eof()) {
        file.read(buffer, buffer_size);
        int bytes_read = file.gcount();
        for (int i = 0; i < bytes_read; ++i) {
          if (counter >= config.lines) return;
          if (buffer[i] == config.delimiter) ++counter;
          std::cout << buffer[i];
        }
      }
    } else {
      file.seekg(0, std::ios::end);
      long long file_size = file.tellg();
      char tmp_buffer[buffer_size];
      file.seekg(0, std::ios::beg);

      long long end_pos = file_size;
      long long bytes_to_read = std::min(file_size, buffer_size);

      while (bytes_to_read != 0) {
        file.seekg(file_size - bytes_to_read);
        file.read(buffer, bytes_to_read);

        for (long long i = bytes_to_read - 1; i > -1; --i) {
          if (counter >= config.lines) return;
          if (buffer[i] == config.delimiter) {
            ++counter;
            long long current_pos = file_size - bytes_to_read + i + 1;
            if (file_size > end_pos) {
              long long tmp_file_size = end_pos - current_pos;
              long long tmp_bytes_to_read = std::min(tmp_file_size, buffer_size);
              while (tmp_bytes_to_read != 0) {
                file.seekg(current_pos);
                file.read(tmp_buffer, tmp_bytes_to_read);
                for (long long j = 0; j < tmp_bytes_to_read; ++j) {
                  std::cout << tmp_buffer[j];
                }
                end_pos = current_pos - 1;
                std::cout << config.delimiter;
                tmp_file_size -= tmp_bytes_to_read;
                tmp_bytes_to_read = std::min(tmp_file_size, buffer_size);
              }
            } else {
              file.seekg(current_pos);
              file.read(tmp_buffer, end_pos - current_pos);
              for (long long j = 0; j < end_pos - current_pos; ++j) {
                std::cout << tmp_buffer[j];
              }
              end_pos = current_pos - 1;
              std::cout << config.delimiter;
            }
          }
        }

        file_size -= bytes_to_read;
        bytes_to_read = std::min(file_size, buffer_size);
      }
      for (long long i = 0; i < end_pos; ++i) {
        std::cout << buffer[i];
      }
    }
  }
};

int main(int argc, char **argv) {
  Config config;
  ArgumentParser parser(argc, argv, config);

  parser.ParseArguments();

  FileReader reader(config.filename, config);

  if (!reader.IsOpen()) {
    parser.ErrorMessage(ArgumentParser::FILE_UNAVAILABLE, config.filename);
  }

  reader.ReadFile();
}
