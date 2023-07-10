#ifndef ASSET_GRADIENT_H
#define ASSET_GRADIENT_H

namespace Asset {

struct gradient
{
  uint32_t Width;
  uint32_t Height;
  uint32_t Pitch;
  void* Memory;
};

}

#endif // ASSET_GRADIENT_H