#include "physics_system.h"

namespace PhysicsSystem {

bool aabb_aabb_test(const v3 aPos,
                    const v3 aSize,
                    const v3 bPos,
                    const v3 bSize,
                    collision& resultCollision)
{
  const v3 positionDelta = aPos - bPos;
  const v3 combinedSize = (aSize + bSize) * 0.5f;

  v3 overlap = V3_ZERO;
  for (int i = 0; i < 3; i++) {
    overlap.E[i] = combinedSize.E[i] - fabs(positionDelta.E[i]);
    if (overlap.E[i] < 0.0f) {
      return false;
    }
  }

  resultCollision = {};

  resultCollision.Penetration = V3_ZERO;
  resultCollision.Point = aPos;
  int axisIndex;
  if (overlap.X < min_f(overlap.Y, overlap.Z)) {
    axisIndex = 0;
  } else if (overlap.Y < overlap.Z) {
    axisIndex = 1;
  } else {
    axisIndex = 2;
  }

  float dSign = sign(positionDelta.E[axisIndex]);
  resultCollision.Penetration.E[axisIndex] = overlap.E[axisIndex] * dSign;
  resultCollision.Point.E[axisIndex] =
    aPos.E[axisIndex] + overlap.E[axisIndex] * dSign;
  resultCollision.Normal.E[axisIndex] = dSign;

  return true;
}

bool slab_test(const v3 rayPos,
               const v3 rayDelta,
               const v3 aabbPos,
               const v3 aabbSize,
               collision& resultCollision)
{
  /**
   * Scaling normalizes edge tests to 0-1, 0 being now and 1 being end of line
   * segment
   */
  const v3 scale = _v3(1.0f / rayDelta.X, 1.0f / rayDelta.Y, 1.0f / rayDelta.Z);
  const v3 signs = _v3(sign(scale.X), sign(scale.Y), sign(scale.Z));

  v3 nearTimes, farTimes;
  for (int i = 0; i < 3; i++) {
    const float boxEdgeNear = aabbPos.E[i] - aabbSize.E[i] * 0.5f * signs.E[i];
    const float boxEdgeFar = aabbPos.E[i] + aabbSize.E[i] * 0.5f * signs.E[i];
    nearTimes.E[i] = (boxEdgeNear - rayPos.E[i]) * scale.E[i];
    farTimes.E[i] = (boxEdgeFar - rayPos.E[i]) * scale.E[i];
  }

  for (int i = 0; i < 3; i++) {
    if (nearTimes.E[i] >
        min_f(farTimes.E[(3 + i - 1) % 3], farTimes.E[(3 + i + 1) % 3])) {
      return false;
    }
  }

  float nearTime = max_f(nearTimes.X, max_f(nearTimes.Y, nearTimes.Z));
  float farTime = min_f(farTimes.X, min_f(farTimes.Y, farTimes.Z));

  /**
   * Neartime > 1.0 == Line starts in front of edge
   * Fartime < 0.0 == Line does not reach edge
   */
  if (nearTime >= 1.0f || farTime <= 0.0f) {
    return false;
  }

  bool enteringFromOutside = nearTime > 0.0f;
  bool exitingFromInside = !enteringFromOutside;

  resultCollision = {};
  resultCollision.Time = max_f(nearTime, 0.0f);
  resultCollision.Normal = V3_ZERO;

  /**
   *  If the near time is greater than zero, the segment starts outside and is
   * entering the box. Otherwise, the segment starts inside the box, and is
   * exiting it.
   */
  for (int i = 0; i < 3; i++) {
    if (enteringFromOutside && nearTimes.E[i] == nearTime) {
      resultCollision.Normal.E[i] = -signs.E[i];
      break;
    }
    if (exitingFromInside && farTimes.E[i] == farTime) {
      resultCollision.Normal.E[i] = -signs.E[i];
      break;
    }
  }

  resultCollision.Penetration = (1.0f - resultCollision.Time) * -rayDelta;
  resultCollision.Point =
    rayPos + rayDelta * resultCollision.Time + resultCollision.Normal * 0.01f;

  return true;
}

bool intersect_ray(const body& target,
                   const body& other,
                   const float hz,
                   collision& resultCollision)
{
  bool noVelocity = true;
  for (size_t i = 0; i < VELOCITY_ELEMENTS; i++) {
    if (abs_f(target.State.Velocity.E[i]) > SLEEP_EPSILON) {
      noVelocity = false;
      break;
    }
  }

  if (noVelocity) {
    return false;
  }

  if (other.Type == PHYSICS_BODY_TYPE_RAY) {
    // Two rays could intersect, but probably not useful to test
    return false;
  }

  if (other.Type == PHYSICS_BODY_TYPE_AABB) {
    return slab_test(target.State.Position,
                     target.State.Velocity * hz,
                     other.State.Position,
                     other.Size,
                     resultCollision);
  }

  return false;
}

bool intersect_aabb(const body& target,
                    const body& other,
                    const float hz,
                    collision& resultCollision)
{
  bool noVelocity = true;
  for (size_t i = 0; i < VELOCITY_ELEMENTS; i++) {
    if (abs_f(target.State.Velocity.E[i]) > SLEEP_EPSILON) {
      noVelocity = false;
      break;
    }
  }

  if (noVelocity) {
    return aabb_aabb_test(target.State.Position,
                          target.Size,
                          other.State.Position,
                          other.Size,
                          resultCollision);
  }

  if (other.Type == PHYSICS_BODY_TYPE_RAY) {
    return false;
  }

  if (other.Type == PHYSICS_BODY_TYPE_AABB) {
    return slab_test(target.State.Position,
                     target.State.Velocity * hz,
                     other.State.Position,
                     target.Size + other.Size,
                     resultCollision);
  }

  return false;
}
}
