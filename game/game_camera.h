#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

#include <algorithm>

#include "game_math.h"

struct game_camera
{
  v2 Angle;
  v3 Position;
  v3 Target;
  v3 TargetLastFrame;
  v3 Velocity;
  float FollowDistance;

  game_entity* TargetEntity;
};

#endif // GAME_CAMERA_H