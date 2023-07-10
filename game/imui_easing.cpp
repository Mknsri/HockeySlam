#include "ui_system.h"

namespace UISystem {

UI_EASING_FUNCTION(EaseOutBack)
{
  const float c1 = 1.70158f;
  const float c3 = c1 + 1.0f;

  return 1.0f + c3 * pow(t - 1.0f, 3.0f) + c1 * pow(t - 1.0f, 2.0f);
}

UI_EASING_FUNCTION(EaseInOutCirc)
{
  return t < 0.5f
           ? (float)(1.0f - sqrt(1.0f - pow(2.0f * t, 2.0f))) / 2.0f
           : (float)(sqrt(1.0f - pow(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;
}

UI_EASING_FUNCTION(EaseInSine)
{
  return 1.0f - cos((t * (float)M_PI) / 2.0f);
}

UI_EASING_FUNCTION(EaseOutQuint)
{
  return 1.0f - pow(1.0f - t, 5.0f);
}

UI_EASING_FUNCTION(EaseInQuad)
{
  return 2.0f * t * t;
}

UI_EASING_FUNCTION(EaseOutQuad)
{
  return 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

UI_EASING_FUNCTION(EaseInOutQuad)
{
  return t < 0.5f ? EaseInQuad(t) : EaseOutQuad(t);
}

UI_EASING_FUNCTION(EaseInOutCubic)
{
  return t < 0.5f ? 4.0f * t * t * t
                  : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

}
