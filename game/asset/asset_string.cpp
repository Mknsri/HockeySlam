#include "asset_string.h"

#ifdef _WIN32
#include <string>
#else
#include <stdio.h>
#endif
namespace Asset {

size_t string_length(const char* const _Value)
{
  size_t len = 0;
  while (_Value[len]) {
    len++;
  }

  return len;
}

string to_string(std::string _Value)
{
  // +1 for null terminator
  size_t characterCount = (_Value.size() + 1);
  char* ptr = (char*)allocate_t(characterCount * sizeof(char));
#if _WIN32
  strcpy_s(ptr, characterCount * sizeof(char), _Value.c_str());
#else
  snprintf(ptr, characterCount, "%s", _Value.c_str());
#endif

  return { ptr };
}

string to_string(const char* _Value)
{
  size_t characterCount = (string_length(_Value) + 1);
  char* ptr = (char*)allocate_t(characterCount * sizeof(char));
#if _WIN32
  strcpy_s(ptr, characterCount * sizeof(char), _Value);
#else
  snprintf(ptr, characterCount, "%s", _Value);
#endif
  return { ptr };
}
}
