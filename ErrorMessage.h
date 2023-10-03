#include <filesystem>

enum class ErrorCodes {
  kFilenameMissing,
  kFileUnavailable,
  kWrongLinesCount,
  kWrongDelimiter,
  kWrongArgument,
  kValueMissing,
  kTailWithoutLines,
};

void PrintHelpMessage(bool is_error, const std::filesystem::path& filepath);

void ErrorMessage(ErrorCodes error_code, char* argument, const std::filesystem::path& filepath);

