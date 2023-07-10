#ifndef ASSET_DDS_H
#define ASSET_DDS_H

#include <assert.h>
#include <stdint.h>
#include <algorithm>

#include "asset_file.h"

namespace Asset {
enum DDS_Texture_Type
{
  DDS_TEXTURE_NONE,
  DDS_TEXTURE_FLAT, // 1D, 2D textures
  DDS_TEXTURE_3D,
  DDS_TEXTURE_CUBEMAP
};

struct DDS_Surface
{
  uint32_t Width;
  uint32_t Height;
  uint32_t Depth;
  uint32_t Size;

  uint8_t* Pixels;
};

struct DDS_Image
{
  uint32_t Format;
  uint32_t Components;
  DDS_Texture_Type Type;
  bool Valid;

  uint32_t SurfaceCount;
  DDS_Surface Surfaces[12];
};
}
#endif // ASSET_DDS_H