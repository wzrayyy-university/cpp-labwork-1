#include <format>
#include <fstream>

#include "ArgumentParser.h"
#include "ErrorMessage.h"
#include "ConsoleReader.h"

int main(int argc, char** argv) {
  Config config;
  ArgumentParser parser(argc, argv, config);

  parser.ParseArguments();

  std::ifstream file(config.filename, std::ios::binary);

  if (!file.is_open()) {
    ErrorMessage(ErrorCodes::kFileUnavailable, config.filename, argv[0]);
  }

  ConsoleReader::ReadFile(file, config.lines, config.tail, config.delimiter);

  return 0;
}
