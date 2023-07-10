#ifndef ASSET_TEXTURE_H
#define ASSET_TEXTURE_H

#include <string>

#include "../game_math.h"
#include "asset_dds.h"

namespace Asset {

enum texture_type
{
  TEXTURE_TYPE_INVALID,
  TEXTURE_TYPE_DDS,
  TEXTURE_TYPE_IMAGE
};

enum texture_wrap_type
{
  TEXTURE_WRAP_CLAMP,
  TEXTURE_WRAP_REPEAT
};

struct image_texture
{
  string Path;
  string MaterialType;
  v2 PixelSize;
  int Components;
  void* Memory;
};

struct texture
{
  string Name;
  texture_type Type;
  union
  {
    image_texture* Image;
    DDS_Image* DDSImage;
  };
  texture_wrap_type WrapType;
};
}

#endif // ASSET_TEXTURE_H
