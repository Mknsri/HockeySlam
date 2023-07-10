#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

struct model_bone;

namespace PhysicsSystem {
static const size_t PHYSICS_ARENA_MAX_BODIES = 100;
static const size_t PHYSICS_ARENA_MAX_COLLISIONS_PER_BODY = 10;

static const float PHYSICS_EPSILON = 1e-6f;
static const float PHYSICS_HZ = 1.0f / 20.0f;
static const float DEFAULT_FRICTION = 0.001f;
static const float DEFAULT_AIRDRAG = 0.001f;
static const float DEFAULT_GRAVITY = -9.807f;
static const float SLEEP_EPSILON = 0.001f;

struct body;

struct state
{
  v3 Position;
  v3 Velocity;
};

static const size_t VELOCITY_ELEMENTS =
  sizeof(((state*)0)->Velocity) / sizeof(((state*)0)->Velocity.E[0]);

enum body_flags
{
  PHYSICS_BODY_FLAG_NONE = 0x0,
  PHYSICS_BODY_FLAG_STATIC = 0x1,
  PHYSICS_BODY_FLAG_TRIGGER = 0x2,
  PHYSICS_BODY_FLAG_ENTITY_CONTROLLED = 0x4
};

enum body_type
{
  PHYSICS_BODY_TYPE_AABB,
  PHYSICS_BODY_TYPE_RAY
};

struct collision
{
  float Time;
  body* Other;
  v3 Normal;
  v3 Point;
  v3 Penetration;
};

struct body
{
#if HOKI_DEV
  const char* Name;
#endif
  state State;
  state PreviousState;
  v3 InterpolatedPosition;

  v3 Size;
  body_type Type;
  uint32_t Flags;

  float Bounce;
  float Friction;
  bool Sleeping;

  size_t CollisionCount;
  collision Collisions[PHYSICS_ARENA_MAX_COLLISIONS_PER_BODY];

  size_t TriggeredCount;

  v3* ParentPosition;
};

struct space
{
  float Accumulator;
  float TimeScale;
  float AirDrag;
  v3 Gravity;

  body Bodies[PHYSICS_ARENA_MAX_BODIES];
  size_t BodyCount;
};

} // namespace PHYSICS_SYSTEM

#endif // PHYSICS_SYSTEM_H