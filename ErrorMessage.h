#include <filesystem>

enum ErrorCodes {
  FILENAME_MISSING,
  FILE_UNAVAILABLE,
  WRONG_LINES_COUNT,
  WRONG_DELIMITER,
  WRONG_ARGUMENT,
  VALUE_MISSING,
  TAIL_WITHOUT_LINES,
};

void PrintHelpMessage(bool is_error, const std::filesystem::path& filepath);

void ErrorMessage(ErrorCodes error_code, char* argument, const std::filesystem::path& filepath);

