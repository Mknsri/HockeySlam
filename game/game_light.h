#ifndef GAME_LIGHT_H
#define GAME_LIGHT_H

#include "game_math.h"
#include "asset/asset_model.h"

enum light_type
{
  LIGHT_TYPE_DIRECTION,
  LIGHT_TYPE_POINT
};

struct game_light
{
  v3 Vector;
  light_type LightType;

  float Constant;
  float Linear;
  float Quadratic;

  v3 Color;
  float Ambient;
  float Diffuse;
  float Specular;
};

#endif // GAME_LIGHT_H