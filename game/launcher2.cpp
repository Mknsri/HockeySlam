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
  return normalize(dir);
}

float get_aim_power(const game_state& state)
{
  float power = 0.0f;
  v2 dir = get_aim_power_dir(state) * 2.0f;

  float towardsUp = dot(normalize(dir), V2_UP);
  if (towardsUp > 0.0f) {
    power = length(dir) * state.SimDelta * 50.0f;
  }

  return clamp(power, 0.0f, 1.1f);
}
v3 get_goal_plane_point(game_state& state)
{
  game_entity& puck = state.Map.Entities.Puck;
  game_entity& goal = state.Map.Entities.Posts;
  v3 goalPos = goal.Body->State.Position;
  v3 puckPos = puck.Body->State.Position;
  // puckPos.X = state.Map.GameCamera.Position.X;

  v2 aimPosition = state.UIContext.TouchPosition;

  v3 goalNormal = _v3(0.0f, 0.0f, 1.0f);
  v3 touchDir = view_to_world(
    state.UIContext.ViewProjection, state.UIContext.RenderContext, aimPosition);

  v3 aimDir = touchDir; // normalize(goalPoint - puckPos);

  v3 w = _v3(goalPos.X, goalPos.Y, goalPos.Z) - puckPos;
  float projection = dot(w, goalNormal) / dot(aimDir, goalNormal);

  aimDir.Y = max_f(0.01f, aimDir.Y);

  return puckPos + projection * aimDir;
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

  const float MAX_LAUNCH_SPEED = 10;

#if HOKI_DEV && 1
  state.DebugAimLine.Position = intersectionOfAimAndGoalPlane;
  state.DebugAimLine.Length = puckPos - intersectionOfAimAndGoalPlane;
#endif

  return initialVel * get_aim_power(state);
}

v3 tick_offense_velocity(game_state& state, v2 aimPosition)
{
  v3 offVel = state.Map.Entities.Offense.Body->State.Velocity;

  const float MAX_X_VEL = 8.5f;
  const float MAX_Z_VEL = -6.0f;
  const float Z_ACCEL = 12.0f;
  const float X_ACCEL = 4.0f;

  // Puckpos goes from 0 (top) to 1 (bottom)
  v2 uiPuckPos = aimPosition;

  state.AimPositionBuffer[state.AimPositionBufferIndex++ %
                          ARRAY_SIZE(state.AimPositionBuffer)] =
    state.UIContext.TouchPosition;

  float addedZVel = 0.3f;
  static const float F_MPI = (float)M_PI;
  float amountToAdd = clamp(1.0f - uiPuckPos.Y, 0.2f, 1.0f);
  addedZVel = amountToAdd * Z_ACCEL * state.SimDelta;

  offVel.Z += -addedZVel;
  offVel.Z = clamp(offVel.Z, MAX_Z_VEL, 0.0f);

  float addedXVel = uiPuckPos.X * X_ACCEL;
  if (sign(offVel.X) != sign(addedXVel)) {
    addedXVel = addedXVel * 4.0f;
  }

  offVel.X += addedXVel * MAX_X_VEL * state.SimDelta;
  offVel.X = clamp(offVel.X, -MAX_X_VEL, MAX_X_VEL);

#if HOKI_DEV && 1
  v3 intersectionOfAimAndGoalPlane = get_goal_plane_point(state);
  state.DebugAimLine.Position = intersectionOfAimAndGoalPlane;
  state.DebugAimLine.Length =
    state.Map.Entities.Puck.Position - intersectionOfAimAndGoalPlane;
#endif

  return offVel;
}
