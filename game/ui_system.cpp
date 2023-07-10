#include "../ogl/ogl_main.h"

#include "debug/debug_strings.cpp"

namespace UISystem {

v3 view_to_world(const mat4x4& uiViewProjection,
                 const render_context* renderContext,
                 const v2& point)
{
  float x = point.X;
  float y = point.Y;

  v4 viewport = mat4x4_invert(uiViewProjection) * _v4(1.0, -1.0, 1.0f, 1.0f);

  // NDC
  x = (2.0f * x) / viewport.X - 1.0f;
  y = (2.0f * y) / viewport.Y - 1.0f;

  // Homogenous clip coordinates
  v4 clip = _v4(x, -y, 1.0f, 1.0f);

  // Inverse projection
  v4 inverseProj = mat4x4_invert(renderContext->ProjectionMatrix) * clip;
  inverseProj = _v4(inverseProj.X, inverseProj.Y, -1.0f, 0.0f);

  // World coordinates
  v4 world = mat4x4_invert(renderContext->ViewMatrix) * inverseProj;

  return normalize(_v3(world.X, world.Y, world.Z));
}
}
