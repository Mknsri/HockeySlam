#ifndef ASSET_DECAL_H
#define ASSET_DECAL_H

#include "asset_texture.h"

namespace Asset {

struct decal
{
  v3 Position;
  v3 Direction;
  texture Texture;
};

}

#endif // ASSET_DECAL_H