#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

#include "asset/asset_texture.h"
#include "asset/asset_model.h"
#include "asset/asset_material.h"
#include "game_math.h"
#include "physics_system.h"
#include "anim_system.h"

static const int MAX_ATTACHED_BODIES = 100;

struct game_entity
{
#if HOKI_DEV
  const char* Name;
#endif
  v3 PreviousPosition;
  v3 Position;
  quat Rotation;
  v3 Scale;

  const Asset::model* Model;
  const Asset::texture* Texture;

  Asset::model_bone* Bones;
  size_t BoneCount;

  mat4x4* BoneTransforms;
  mat4x4* BoneWorldPositions;

  PhysicsSystem::body* Body;
  v3 BodyOffset;

  AnimationSystem::run_id ActiveAnimations[10];
  size_t ActiveAnimationCount;
};

struct instanced_entity : game_entity
{
  int InstanceCount;
  v3 InstanceSpacing;
};

#endif // GAME_ENTITY_H