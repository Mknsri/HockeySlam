#ifndef ANIM_SYSTEM_H
#define ANIM_SYSTEM_H

#include "asset/asset_anim.h"

struct game_entity;
namespace AnimationSystem {
using Asset::animation;
using Asset::animation_channel;
using Asset::animation_keyframe;
using Asset::animation_path_type;
using Asset::model_bone;
using Asset::model_node;

static const size_t ANIMATION_LOOPS_INFINITE = SIZE_MAX;
static const float ANIMATION_FULL_WEIGHT = 1.0f;
static const float ANIMATION_DEFAULT_SPEED = 1.0f;
static const uint16_t ANIMATION_STACK_MAX_SIZE = 16;
static const size_t ANIMATOR_MAX_ANIMATIONS = 64;
static const size_t ANIMATION_MAX_CHANNELS = 64;
static const uint32_t ANIMATION_NO_ID = 0;

static const animation EMPTY_ANIMATION = { "EMPTY", nullptr, 0, 0.0f };

enum class animation_loop_style
{
  NO_LOOP,
  WRAP,
  PING_PONG,
};

enum class animation_driver
{
  TIME,
  INPUT
};

struct animation_update_transform
{
  v3 NewTranslation;
  quat NewRotation;
  v3 NewScale;
  uint32_t BoneIndex;
  Asset::animation_path_type PathType;
};

typedef uint32_t run_id;

struct animation_run
{
  run_id Id;
  const animation* Animation;
  float StartDelay;
  float CurrentTime;
  uint32_t* ChannelKeyframes;
  animation_update_transform* ChannelTransforms;
  size_t ChannelCount;
  float Speed;
  size_t Loops;
  animation_loop_style LoopStyle;
  float Weight;
  uint32_t EntityCount;
  animation_driver Driver;
};

struct animation_group
{
  const animation* Animations[ANIMATION_STACK_MAX_SIZE];
  float Weights[ANIMATION_STACK_MAX_SIZE];
  uint16_t Size;
  bool UpdateWeightsOnly;
  animation_loop_style LoopStyle;
};

struct animator_update
{
  float DeltaTime;
  animation_run* Run;
};

struct animator
{
  float DeltaTime;
  float DeltaInput;
  animation_run RunningAnimations[ANIMATOR_MAX_ANIMATIONS];
  animator_update FrameData[ANIMATOR_MAX_ANIMATIONS];
};

}

#endif // ANIM_SYSTEM_H