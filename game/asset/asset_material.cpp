#include "asset_material.h"

namespace Asset {
material create_default_material()
{
  material result = {};
  result.Shine = 32.0f;

  return result;
}
}