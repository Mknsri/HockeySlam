#include "asset_file.h"

#include <algorithm>
#include <string>

#include "asset_string.h"

namespace Asset {

file load_file(std::string path, game_memory& gameMemory)
{
  file result;

  // Get filetype
  std::size_t lastPathSeparator = path.find_last_of("/\\");
  std::size_t lastDotSeparator = path.find_last_of(".");
  const std::string dir = path.substr(0, lastPathSeparator);
  std::string type = path.substr(lastDotSeparator + 1);
  std::transform(type.begin(), type.end(), type.begin(), [](char c) {
    return static_cast<char>(::tolower(c));
  });
  std::string fileName = path.substr(lastPathSeparator + 1);
  std::transform(fileName.begin(),
                 fileName.end(),
                 fileName.begin(),
                 [](char c) { return static_cast<char>(::tolower(c)); });

  result.FileExt = to_string(type);
  result.Directory = to_string(dir);
  result.FileName = to_string(fileName);

  result.Path = to_string(path);
  result.Size = gameMemory.GetFileSize(path.c_str());
  HOKI_ASSERT(result.Size);
  result.Memory = allocate(result.Size + 1);

  uint8_t* startOfFile = (uint8_t*)result.Memory;
  gameMemory.ReadFile(path.c_str(), startOfFile);
  // Add null byte
  *((uint8_t*)result.Memory + result.Size) = 0x0;

  return result;
}
}