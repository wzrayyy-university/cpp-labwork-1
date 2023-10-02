#include <iostream>
#include "ErrorMessage.h"

void PrintHelpMessage(bool is_error, const std::filesystem::path& filepath) {
  const char options_text[] = "Options:\n"
                              "\t-l, --lines=N\t\tPrints N lines to stdout.\n"
                              "\t-t, --tail\t\tReverse printing direction.\n"
                              "\t-d --delimiter=CHAR\tSet delimiter character to CHAR.\n";

  if (is_error) {
    std::cerr << "\"Usage:\\n\"\n";
    std::cerr << "\t " << filepath << R"( [OPTIONS] [FILE]\n\n")";
    std::cerr << options_text;
  } else {
    std::cout << "\"Usage:\\n\"\n";
    std::cout << "\t " << filepath << R"( [OPTIONS] [FILE]\n\n")";
    std::cout << options_text;
  }
};

void ErrorMessage(ErrorCodes error_code, char* argument, const std::filesystem::path& filepath) {
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
  PrintHelpMessage(true, filepath);
  exit(EXIT_FAILURE);
};

