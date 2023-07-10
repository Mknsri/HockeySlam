#ifndef ASSET_ANIM_H
#define ASSET_ANIM_H

#include "../game_math.h"
#include "asset_string.h"
#include "asset_model.h"

namespace Asset {

struct animation_keyframe
{
  float StartTime;
  v3 Translation;
  quat Rotation;
  v3 Scale;
};

enum animation_path_type
{
  TRANSFORMATION,
  TRANSLATION,
  ROTATION,
  SCALE
};

struct animation_channel
{
  uint32_t BoneIndex;
  size_t KeyframeCount;
  animation_keyframe* Keyframes;
  animation_path_type PathType;
  float Duration;
};

struct animation
{
  string Name;
  animation_channel* Channels;
  size_t ChannelCount;
  float Duration;
};
}

#endif // ASSET_ANIM_H