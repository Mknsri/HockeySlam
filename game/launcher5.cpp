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

v2 get_aim_power_dir(const game_state& state)
{
  v2 prev = state.AimPositionBuffer[state.AimPositionBufferIndex %
                                    ARRAY_SIZE(state.AimPositionBuffer)];
  v2 dir = _v2(0.0f);
  for (size_t i = 1; i < ARRAY_SIZE(state.AimPositionBuffer); i++) {
    int index =
      (state.AimPositionBufferIndex + i) % ARRAY_SIZE(state.AimPositionBuffer);
    dir += (prev - state.AimPositionBuffer[index]);
    prev = state.AimPositionBuffer[index];
  }
  return dir;
}

v3 get_goal_plane_point(game_state& state)
{
  const v3 puckPos = state.Map.Entities.Puck.Position;
  game_entity& goal = state.Map.Entities.Posts;
  v3 goalPos = goal.Body->State.Position;

  float delta = state.AimPosition.X - state.AimDir.X;
  if (state.AimPosition.Y < 0.0f) {
    state.AimDir.X += delta * 0.04f;
  } else {
    state.AimDir.X += delta * 0.01f;
  }

  float xRange = goal.Body->Size.X * 1.5f;
  float yRange = 9.0f * get_aim_power_dir(state).Y;
  v2 aimPos =
    hadamard_multiply(_v2(state.AimDir.X, 1.0f), _v2(-xRange, yRange));

  v3 goalNormal = _v3(0.0f, 0.0f, 1.0f);

  v3 aimDir = _v3(aimPos.X, aimPos.Y, goalPos.Z) - puckPos;

  v3 w = _v3(goalPos.X, goalPos.Y, goalPos.Z) - puckPos;
  float projection = dot(w, goalNormal) / dot(aimDir, goalNormal);

  return puckPos + projection * aimDir;
}

void do_circle(game_state& state)
{
  if (state.Phase == game_phase::REPLAYING) {
    state.Map.Entities.Arrow.Scale = _v3(0.0f);
    return;
  }

  const v3 puckPos = offense_puck_shoot_offset(state);
  v3 intersectionOfAimAndGoalPlane = get_goal_plane_point(state);
  v3 distanceToGoal = intersectionOfAimAndGoalPlane - puckPos;
  float angle = 90.0f + rad_to_deg(atan2(distanceToGoal.X, distanceToGoal.Z));
  state.Map.Entities.Arrow.Scale = _v3(distanceToGoal.Z * 0.05f);
  state.Map.Entities.Arrow.Rotation = quat_from_euler(0.0f, angle, 180.0f);
  state.Map.Entities.Arrow.Position =
    intersectionOfAimAndGoalPlane - _v3(0.0f, 0.0f, -2.0f);
}

v3 get_shoot_velocity(game_state& state)
{
  game_entity& goal = state.Map.Entities.Posts;
  game_entity& offense = state.Map.Entities.Offense;

  // Shoot
  v3 goalPos = goal.Body->State.Position;
  const v3 puckPos = offense_puck_shoot_offset(state);

  state.TimeAtLaunch = state.SimTime;

  v3 intersectionOfAimAndGoalPlane = get_goal_plane_point(state);

  v3 displacement = (intersectionOfAimAndGoalPlane - puckPos);
  v3 acceleration = state.PhysicsSpace.Gravity;

  v3 initialVel =
    kinematic_get_initial_velocity(displacement, 0.5f, acceleration);

#if HOKI_DEV && 1
  state.DebugAimLine.Position = intersectionOfAimAndGoalPlane;
  state.DebugAimLine.Length = puckPos - intersectionOfAimAndGoalPlane;
#endif
  do_circle(state);

  return initialVel;
}

v3 tick_offense_velocity(game_state& state, v2 aimPos)
{
  state.AimPositionBuffer[state.AimPositionBufferIndex++ %
                          ARRAY_SIZE(state.AimPositionBuffer)] =
    state.UIContext.TouchPosition;

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
  // Tick aimpower
  if (aimPos.Y > 0.0f) {
    state.AimPower += (0.03f * aimPos.Y);
  } else {
    state.AimPower -= 0.04f;
  }
  state.AimPower = clamp(state.AimPower, 0.0f, 1.0f);

  offVel.X += addedXVel * MAX_X_VEL;
  offVel.X = clamp(offVel.X, -MAX_X_VEL, MAX_X_VEL);

#if HOKI_DEV && 1
  state.DebugAimLine.Position = intersectionOfAimAndGoalPlane;
  state.DebugAimLine.Length =
    offense_puck_shoot_offset(state) - intersectionOfAimAndGoalPlane;
#endif

  do_circle(state);

  // return _v3(0.0f);
  return offVel;
}
