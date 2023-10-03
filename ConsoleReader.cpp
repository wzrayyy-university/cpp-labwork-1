#include <iostream>

#include "ConsoleReader.h"
#include "ArgumentParser.h"

void ConsoleReader::ReadFileToStdout(std::ifstream& file, const uint64_t& lines, const char& delimiter) {
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

int64_t ConsoleReader::FindDelimiter(std::ifstream& file, const uint64_t& lines, const char& delimiter) {
  int64_t counter = 0;

  file.seekg(-1, std::ios::end);
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

void ConsoleReader::ReadFile(std::ifstream& file, const uint64_t& lines, const bool& tail, const char& delimiter) {
  if (tail) {
    int64_t pos = FindDelimiter(file, lines, delimiter);
    file.seekg(pos);
    ReadFileToStdout(file, kDefaultLinesValue, kDefaultDelimiter);
    exit(EXIT_SUCCESS);
  }
  ReadFileToStdout(file, lines, delimiter);
};