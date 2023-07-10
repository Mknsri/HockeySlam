#ifndef ASSET_FILE_H
#define ASSET_FILE_H

#include "asset_string.h"

namespace Asset {

struct file
{
  string Path;
  string FileExt;
  string Directory;
  string FileName;
  size_t Size;

  void* Memory;
};

}
#endif