#ifndef ASSET_MATERIAL_H
#define ASSET_MATERIAL_H

#include "../game_math.h"

namespace Asset {
struct material
{
  float Shine;
  bool Translucent;

  v3 Albedo;
  texture* AlbedoMap;

  float Roughness;
  texture* RoughnessMap;

  float Metallic;
  texture* MetallicMap;

  texture* NormalMap;

  float AmbientOcclusion;
};
}

#endif // ASSET_MATERIAL_H