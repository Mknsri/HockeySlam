#include "physics_system.h"

#include <float.h>

#include "physics_body_creations.cpp"
#include "physics_intersection_tests.cpp"

namespace PhysicsSystem {
space create_space()
{
  space result = {};
  result.Accumulator = 0;
  result.BodyCount = 0;
  result.TimeScale = 1.0f;
  result.AirDrag = 0.0f; // DEFAULT_AIRDRAG;
  result.Gravity = _v3(0.0f, PhysicsSystem::DEFAULT_GRAVITY, 0.0f);

  return result;
}

void reset_space(space& space)
{
  space.BodyCount = 0;
  space.Accumulator = 0;
}

void add_velocity(body* target, const v3 v)
{
  target->Sleeping = false;
  target->State.Velocity += v;
}

void set_velocity(body* target, const v3 v)
{
  target->Sleeping = false;
  target->State.Velocity = v;
}

void add_position(body* target, const v3 v)
{
  target->Sleeping = false;
  target->State.Position += v;
}

void set_position(body* target, const v3 v)
{
  target->Sleeping = false;
  target->State.Position = v;
}

bool collided(const body* a, const body* b)
{
  for (size_t i = 0; i < a->CollisionCount; i++) {
    if (a->Collisions[i].Other == b) {
      return true;
    }
  }

  return false;
}

collision collision_resolution(const body& target,
                               space& physicsSpace,
                               const float hz)
{
  collision closestCollision = {};
  closestCollision.Time = FLT_MAX;

  float triggerCollisionTime = FLT_MAX;
  body* triggerOther = nullptr;
  for (size_t b = 0; b < physicsSpace.BodyCount; b++) {
    body& other = physicsSpace.Bodies[b];
    if (&other == &target) {
      // Dont collide with itself
      continue;
    }

    collision collisionResult = {};
    bool collided = false;
    switch (target.Type) {
      case PHYSICS_BODY_TYPE_AABB:
        collided = intersect_aabb(target, other, hz, collisionResult);
        break;

      case PHYSICS_BODY_TYPE_RAY:
        collided = intersect_ray(target, other, hz, collisionResult);
        break;

      default:
        HOKI_ASSERT(false);
        break;
    }

    if (collided) {
      if (collisionResult.Time < closestCollision.Time) {
        // Trigger collisions are not recorded except by flagging the trigger
        if (other.Flags & PHYSICS_BODY_FLAG_TRIGGER) {
          triggerCollisionTime = collisionResult.Time;
          triggerOther = &other;
          continue;
        }
        collisionResult.Other = &other;
        closestCollision = collisionResult;
      }
    }
  }

  if (triggerCollisionTime < closestCollision.Time) {
    triggerOther->TriggeredCount++;
  }

  return closestCollision;
}

void tick(space& physicsSpace, const float hz)
{
  for (size_t b = 0; b < physicsSpace.BodyCount; b++) {
    body& body = physicsSpace.Bodies[b];

    body.PreviousState = body.State;
    if (body.ParentPosition != nullptr) {
      body.State.Position = *body.ParentPosition;
    }

    if (body.Sleeping || (body.Flags & PHYSICS_BODY_FLAG_STATIC) ||
        (body.Flags & PHYSICS_BODY_FLAG_ENTITY_CONTROLLED)) {
      continue;
    };

    // Gravity is per second, so we need to apply it only for this tick
    body.State.Velocity +=
      physicsSpace.Gravity * hz * (1.0f - physicsSpace.AirDrag);

    // collision_resolution(body, physicsSpace, hz);

    float timeLeft = hz;
    body.CollisionCount = 0;
    while (timeLeft > 0.0f) {
      collision closestCollision =
        collision_resolution(body, physicsSpace, timeLeft);
      if (closestCollision.Time < FLT_MAX) {
        if (body.CollisionCount == PHYSICS_ARENA_MAX_COLLISIONS_PER_BODY) {
          body.CollisionCount = 0;
        }
        body.Collisions[body.CollisionCount++] = closestCollision;

        body.State.Velocity =
          reflect(body.State.Velocity, closestCollision.Normal);

        for (int v = 0; v < VELOCITY_ELEMENTS; v++) {
          if (closestCollision.Normal.E[v] == 0.0f) {
            body.State.Velocity.E[v] =
              body.State.Velocity.E[v] * (1.0f - body.Friction);
          } else {
            body.State.Velocity.E[v] = body.State.Velocity.E[v] * body.Bounce;
          }
        }

        body.State.Position = closestCollision.Point;
        if (closestCollision.Time == 0.0f) {
          break;
        }

        timeLeft -= closestCollision.Time * hz;
      } else {
        // Exit loop
        break;
      }
    }

    body.State.Position += body.State.Velocity * timeLeft;

    for (size_t i = 0; i < VELOCITY_ELEMENTS; i++) {
      if (abs_f(body.State.Velocity.E[i]) < (SLEEP_EPSILON)) {
        body.State.Velocity.E[i] = 0.0f;
      } else {
        body.Sleeping = false;
        break;
      }
      // A dynamic body can only be sleeping if it is resting(colliding) on
      // something
      body.Sleeping = body.CollisionCount > 0;
    }
  }
}

void simulate(space& physicsSpace, const float deltaTime)
{
  physicsSpace.Accumulator += deltaTime;

  while (physicsSpace.Accumulator > PHYSICS_HZ) {
    tick(physicsSpace, PHYSICS_HZ);
    physicsSpace.Accumulator -= PHYSICS_HZ;
  }

  for (size_t i = 0; i < physicsSpace.BodyCount; i++) {
    float alpha = physicsSpace.Accumulator / PHYSICS_HZ;
    body& body = physicsSpace.Bodies[i];

    v3 interpolateFrom = body.PreviousState.Position;
    v3 interpolateTo = body.State.Position;
    if (!body.Sleeping && body.CollisionCount > 0) {
      collision lastCollision = body.Collisions[body.CollisionCount - 1];
      if (alpha <= lastCollision.Time) {
        float remainder = lastCollision.Time;
        alpha /= remainder;
        interpolateTo = lastCollision.Point;
      } else {
        float remainder = 1.0f - lastCollision.Time;
        alpha = (alpha - lastCollision.Time) / remainder;
        body.PreviousState.Position = lastCollision.Point;
        body.PreviousState.Velocity = body.State.Velocity;
        interpolateFrom = lastCollision.Point;
      }
    }

    body.InterpolatedPosition =
      ((1.0f - alpha) * interpolateFrom) + ((alpha)*interpolateTo);
  }
}
}