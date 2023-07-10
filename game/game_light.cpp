#include "game_light.h"

/**
 * http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
 * Radius - Constant - Linear - Quadratic
 */
const static int POINT_LIGHT_ELEMENTS = 4;
const static float POINT_LIGHT_VALUES[]{
  7.0f,   1.0f, 0.7f,   1.8f,    13.0f,   1.0f, 0.35f,   0.44f,
  20.0f,  1.0f, 0.22f,  0.20f,   32.0f,   1.0f, 0.14f,   0.07f,
  50.0f,  1.0f, 0.09f,  0.032f,  65.0f,   1.0f, 0.07f,   0.017f,
  100.0f, 1.0f, 0.045f, 0.0075f, 160.0f,  1.0f, 0.027f,  0.0028f,
  200.0f, 1.0f, 0.022f, 0.0019f, 325.0f,  1.0f, 0.014f,  0.0007f,
  600.0f, 1.0f, 0.007f, 0.0002f, 3250.0f, 1.0f, 0.0014f, 0.000007f
};

v3 choose_nearest_point_light_index(const float radius)
{
  float min = 0.0f;
  int index = 0;
  const int lastIndex = (ARRAY_SIZE(POINT_LIGHT_VALUES)) - POINT_LIGHT_ELEMENTS;
  while (index < lastIndex) {
    if (radius >= min && radius <= POINT_LIGHT_VALUES[index]) {
      break;
    }
    min = POINT_LIGHT_VALUES[index];
    index += POINT_LIGHT_ELEMENTS;
  }

  return _v3(POINT_LIGHT_VALUES[index + 1],
             POINT_LIGHT_VALUES[index + 2],
             POINT_LIGHT_VALUES[index + 3]);
  ;
}

game_light create_directional_light(const v3 direction,
                                    const v3 color,
                                    float ambient,
                                    float diffuse,
                                    float specular)
{
  game_light result = {};
  result.Vector = normalize(direction);
  result.Color = color;
  result.LightType = LIGHT_TYPE_DIRECTION;
  result.Ambient = ambient;
  result.Diffuse = diffuse;
  result.Specular = specular;

  return result;
}

game_light create_point_light(const v3 position,
                              const float radius,
                              const v3 color,
                              float ambient,
                              float diffuse,
                              float specular)
{
  game_light result = {};
  result.Vector = position;
  result.LightType = LIGHT_TYPE_POINT;
  result.Color = color;
  result.Ambient = ambient;
  result.Diffuse = diffuse;
  result.Specular = specular;

  v3 pointLightValues = choose_nearest_point_light_index(radius);
  result.Constant = pointLightValues.E[0];
  result.Linear = pointLightValues.E[1];
  result.Quadratic = pointLightValues.E[2];

  return result;
}