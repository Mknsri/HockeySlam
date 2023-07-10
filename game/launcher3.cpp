#include "../ogl/ogl_main.h"

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

float get_aim_power(const game_state& state)
{
  return clamp(state.AimPower, 0.0f, 1.1f);
}

v3 get_goal_plane_point(game_state& state)
{
  game_entity& puck = state.Map.Entities.Puck;
  game_entity& goal = state.Map.Entities.Posts;
  v3 goalPos = goal.Body->State.Position;
  v3 puckPos = puck.Body->State.Position;

  float xRange = goal.Body->Size.X * 2.0f;
  float yRange = 8.0f * get_aim_power(state);
  v2 aimPos =
    hadamard_multiply(_v2(state.AimPosition.X, 1.0f), _v2(-xRange, yRange));

  v3 goalNormal = _v3(0.0f, 0.0f, 1.0f);

  v3 aimDir = _v3(aimPos, goalPos.Z) - puckPos;

  v3 w = _v3(goalPos.X, goalPos.Y, goalPos.Z) - puckPos;
  float projection = dot(w, goalNormal) / dot(aimDir, goalNormal);

  return puckPos + projection * aimDir;
}

void do_arrow(game_state& state)
{
  if (state.AimPower < 0.1f) {
    state.Map.Entities.Arrow.Scale = _v3(0.0f);
    return;
  }

  v3 puckPos = state.Map.Entities.Puck.Position;
  v3 intersectionOfAimAndGoalPlane = get_goal_plane_point(state);
  v3 distanceToGoal = intersectionOfAimAndGoalPlane - puckPos;
  float angle =
    180.0f + rad_to_deg(atan2(distanceToGoal.X, distanceToGoal.Z - 2.0f));
  state.Map.Entities.Arrow.Scale = _v3(-1.0f, -1.0f, distanceToGoal.Z * 0.08f);
  state.Map.Entities.Arrow.Rotation = quat_from_euler(0.0f, -angle, 45.0f);
  state.Map.Entities.Arrow.Position =
    intersectionOfAimAndGoalPlane - _v3(0.0f, 0.0f, -2.0f);
}

v3 get_shoot_velocity(game_state& state)
{
  game_entity& puck = state.Map.Entities.Puck;
  game_entity& goal = state.Map.Entities.Posts;
  game_entity& offense = state.Map.Entities.Offense;

  // Shoot
  v3 goalPos = goal.Body->State.Position;
  v3 puckPos = puck.Body->State.Position;

  state.TimeAtLaunch = state.SimTime;

  v3 intersectionOfAimAndGoalPlane = get_goal_plane_point(state);

  v3 displacement = (intersectionOfAimAndGoalPlane - puck.Position);
  v3 acceleration = state.PhysicsSpace.Gravity;

  v3 initialVel =
    kinematic_get_initial_velocity(displacement, 0.5f, acceleration);

#if HOKI_DEV && 1
  state.DebugAimLine.Position = intersectionOfAimAndGoalPlane;
  state.DebugAimLine.Length = puckPos - intersectionOfAimAndGoalPlane;
#endif
  do_arrow(state);

  return initialVel;
}

v3 tick_offense_velocity(game_state& state, v2 aimPos)
{
  v3 offVel = state.Map.Entities.Offense.Body->State.Velocity;

  const float MAX_X_VEL = 8.5f;
  const float MAX_Z_VEL = -6.0f;
  const float Z_ACCEL = 12.0f;
  const float X_ACCEL = 4.0f;

  float addedZVel = 0.3f;
  static const float F_MPI = (float)M_PI;
  float amountToAdd = clamp(1.0f - aimPos.Y, 0.2f, 1.0f);
  addedZVel = amountToAdd * Z_ACCEL * state.SimDelta;

  offVel.Z += -addedZVel;
  offVel.Z = clamp(offVel.Z, MAX_Z_VEL, 0.0f);

  float addedXVel = aimPos.X * X_ACCEL * state.SimDelta;
  if (sign(offVel.X) != sign(addedXVel)) {
    addedXVel = addedXVel * 4.0f;
  }

  if (aimPos.Y > 0.0f) {
    addedXVel = 0.0f;
  }
  v3 intersectionOfAimAndGoalPlane = get_goal_plane_point(state);

  offVel.X += addedXVel * MAX_X_VEL;
  offVel.X = clamp(offVel.X, -MAX_X_VEL, MAX_X_VEL);

#if HOKI_DEV && 1
  state.DebugAimLine.Position = intersectionOfAimAndGoalPlane;
  state.DebugAimLine.Length =
    state.Map.Entities.Puck.Position - intersectionOfAimAndGoalPlane;
#endif

  do_arrow(state);

  // Tick aimpower
  if (aimPos.Y > 0.0f) {
    state.AimPower += 0.01f;
  } else {
    state.AimPower -= 0.02f;
  }
  state.AimPower = clamp(state.AimPower, 0.0f, 1.0f);

  // return _v3(0.0f);
  return offVel;
}
