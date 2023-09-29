#include <iostream>
#include <cstdint>
#include <format>
#include <cstring>
#include <fstream>

const char kDefaultDelimiter = '\n';

struct Config {
  char *filename{};
  uint64_t lines = -1;
  bool tail = false;
  char delimiter = kDefaultDelimiter;
};

class ArgumentParser {
 private:
  Config &config;
  int argc;
  char **argv;
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

  void GetLines(char arg[], int &i) {
    char *temp_lines;
    int lines;
    if (strncmp(arg, "--lines=", strlen("--lines=")) == 0) {
      temp_lines = arg + strlen("--lines=");
    } else if (strncmp(arg, "-l=", strlen("-l=")) == 0) {
      temp_lines = arg + strlen("-l=");
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

  void GetDelimiter(char arg[], int &i) {
    char *value;
    if (strncmp(arg, "--delimiter=", strlen("--delimiter=")) == 0) {
      value = arg + strlen("--delimiter=");
    } else if (strncmp(arg, "-d=", strlen("-d=")) == 0) {
      value = arg + strlen("-d=");
    } else {
      if (argc > ++i)
        value = argv[i];
      else
        ErrorMessage(ErrorCodes::VALUE_MISSING, arg);
    }

    if (strncmp(value, "'", 1) == 0) {
      if (strlen(value) == 3) {
        config.delimiter = value[1];
      } else if (strlen(value) == 4) {
        config.delimiter = GetSpecialChar(value + strlen("\'"));
      } else {
        ErrorMessage(ErrorCodes::WRONG_DELIMITER, value);
      }
    } else if (strlen(value) == 1) {
      config.delimiter = value[0];
    } else if (strlen(value) == 2) {
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
      if (strncmp(argv[i], "-", 1) == 0) {
        if (strncmp(argv[i], "-h", 2) == 0 || strncmp(argv[i], "--help", 6) == 0) {
          PrintHelpMessage(false);
          exit(0);
        } else if (strncmp(argv[i], "-t", 2) == 0 || strncmp(argv[i], "--tail", strlen("--tail")) == 0) {
          if (!is_lines_set) {
            ErrorMessage(ErrorCodes::TAIL_WITHOUT_LINES, argv[i]);
          }
          config.tail = true;
        } else if (strncmp(argv[i], "-d", 2) == 0 || strncmp(argv[i], "--lines", strlen("--delimiter")) == 0) {
          GetDelimiter(argv[i], i);
        } else if (strncmp(argv[i], "-l", 2) == 0 || strncmp(argv[i], "--lines", strlen("--lines")) == 0) {
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

class FileReader {
 private:
  Config config;
  std::ifstream file;

  void ReadFileHead() {
    long long counter = 0;
    constexpr long long buffer_size = 1024 * 1024;
    char buffer[buffer_size];

    while (!file.eof()) {
      file.read(buffer, buffer_size);
      std::streamsize bytes_read = file.gcount();
      for (int i = 0; i < bytes_read; ++i) {
        if (counter >= config.lines) return;
        if (buffer[i] == config.delimiter) ++counter;
        std::cout << buffer[i];
      }
    }
  };

  void ReadFileTail() { // прости...
    long long counter = 0;

    constexpr long long buffer_size = 1024 * 512;
    char buffer[buffer_size];
    char tmp_buffer[buffer_size];

    file.seekg(0, std::ios::end);
    long long file_size = file.tellg();
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
  };
 public:
  FileReader(char filename[], Config &config) : config(config) {
    file.open(filename);
  };

  bool IsOpen() {
    return file.is_open();
  }

  void ReadFile() {
    if (!config.tail) {
      ReadFileHead();
    } else {
      ReadFileTail();
    }
  };
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
