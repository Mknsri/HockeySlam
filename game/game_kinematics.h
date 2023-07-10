#ifndef GAME_KINEMATICS_H
#define GAME_KINEMATICS_H

#include "game_math.h"

v3 kinematic_get_initial_velocity(const v3 displacement,
                                  const float time,
                                  const v3 acceleration)
{
  return (2.0f * displacement - acceleration * (time * time)) / (2.0f * time);
}

float kinematic_get_time(const v3 displacement,
                         const v3 initialVelocity,
                         const v3 acceleration)
{
  const float U = length(initialVelocity);
  const float A = length(acceleration);
  const float S = length(displacement);
  return (sqrtf(2.0f * A * S + U * U) - U) / A;
}

#endif