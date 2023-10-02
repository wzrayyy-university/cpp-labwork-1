#include <cstring>

#include "ArgumentParser.h"
#include "ErrorMessage.h"

char ArgumentParser::GetSpecialChar(char* ch) {
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
    ErrorMessage(ErrorCodes::kWrongDelimiter, ch, argv[0]);
    return '\0';
  }
};

void ArgumentParser::GetLines(char arg[], int& idx) {
  char* temp_lines;
  int lines;
  if (strncmp(arg, ArgumentsWithValue::kLines, strlen(ArgumentsWithValue::kLines)) == 0) {
    temp_lines = arg + strlen(ArgumentsWithValue::kLines);
  } else if (strncmp(arg, ArgumentsWithValue::kLinesShort, strlen(ArgumentsWithValue::kLinesShort)) == 0) {
    temp_lines = arg + strlen(ArgumentsWithValue::kLinesShort);
  } else {
    if (argc > ++idx) {
      temp_lines = argv[idx];
    } else {
      ErrorMessage(ErrorCodes::kValueMissing, arg, argv[0]);
    }
  }
  try {
    lines = std::stoi(temp_lines);
  } catch (std::invalid_argument& e) {
    ErrorMessage(ErrorCodes::kWrongLinesCount, temp_lines, argv[0]);
  }

  if (lines >= 0) {
    config.lines = lines;
  } else {
    ErrorMessage(ErrorCodes::kWrongLinesCount, temp_lines, argv[0]);
  }
};

void ArgumentParser::GetDelimiter(char arg[], int& i) {
  char* value;
  if (strncmp(arg, ArgumentsWithValue::kDelimiter, strlen(ArgumentsWithValue::kDelimiter)) == 0) {
    value = arg + strlen(ArgumentsWithValue::kDelimiter);
  } else if (strncmp(arg, ArgumentsWithValue::kDelimiterShort, strlen(ArgumentsWithValue::kDelimiterShort)) == 0) {
    value = arg + strlen(ArgumentsWithValue::kDelimiterShort);
  } else {
    if (argc > ++i)
      value = argv[i];
    else
      ErrorMessage(ErrorCodes::kValueMissing, arg, argv[0]);
  }

  if (strncmp(value, "'", strlen("\'")) == 0) {
    if (strlen(value) == strlen(R"('*')")) {
      config.delimiter = value[1];
    } else if (strlen(value) == strlen(R"('\*')")) {
      config.delimiter = GetSpecialChar(value + strlen("\'"));
    } else {
      ErrorMessage(ErrorCodes::kWrongDelimiter, value, argv[0]);
    }
  } else if (strlen(value) == strlen("*")) {
    config.delimiter = value[0];
  } else if (strlen(value) == strlen(R"(\*)")) {
    config.delimiter = GetSpecialChar(value);
  } else {
    ErrorMessage(ErrorCodes::kWrongDelimiter, value, argv[0]);
  }
};

ArgumentParser::ArgumentParser(int argc, char** argv, Config& config) :
    config(config), argc(argc), argv(argv) {
  this->is_lines_set = false;
  this->is_filename_set = false;
};

void ArgumentParser::ParseArguments() {
  for (int i = 1; i < argc; ++i) {
    if (strncmp(argv[i], "-", 1) == 0) {
      if (strncmp(argv[i], "-h", 2) == 0 || strncmp(argv[i], "--help", 6) == 0) {
        PrintHelpMessage(false, argv[0]);
        exit(0);
      } else if (strncmp(argv[i], Arguments::kTailShort, 2) == 0
          || strncmp(argv[i], Arguments::kTail, strlen(Arguments::kTail)) == 0) {
        if (!is_lines_set) {
          ErrorMessage(ErrorCodes::kTailWithoutLines, argv[i], argv[0]);
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
        ErrorMessage(ErrorCodes::kWrongArgument, argv[i], argv[0]);
      }
    } else {
      is_filename_set = true;
      config.filename = argv[i];
    }
  }
  if (!is_filename_set) {
    ErrorMessage(ErrorCodes::kFilenameMissing, reinterpret_cast<char*>('\0'), argv[0]);
  }
}