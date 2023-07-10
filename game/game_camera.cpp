#include "game_camera.h"

#include "game_entity.h"

static game_camera create_camera(v3 position, v3 target)
{
  game_camera result = {};

  result.Position = position;
  result.Target = target;
  result.TargetLastFrame = result.Target;
  result.FollowDistance = length(position - result.Target);

  v3 v = result.Target - position;
  float vLength = length(v);
  float pitch = asin(v.Y / vLength);
  float yaw = atan2(v.Z, v.X);

  result.Angle = _v2(yaw, pitch);

  return result;
}

static game_camera create_camera(v3 position, game_entity* targetEntity)
{
  game_camera result = {};

  result.Position = position;
  result.TargetEntity = targetEntity;
  result.Target = targetEntity->Position;
  result.TargetLastFrame = result.Target;
  result.FollowDistance = length(position - result.Target);

  v3 v = result.Target - position;
  float vLength = length(v);
  float pitch = asin(v.Y / vLength);
  float yaw = atan2(v.Z, v.X);

  result.Angle = _v2(yaw, pitch);

  return result;
}

static game_camera create_camera(v3 position,
                                 float yawDegrees,
                                 float pitchDegrees)
{
  game_camera result = {};

  result.Position = position;
  result.Angle = _v2(deg_to_rad(yawDegrees), deg_to_rad(pitchDegrees));
  result.Target.X = cos(result.Angle.X) * cos(result.Angle.Y);
  result.Target.Y = sin(result.Angle.Y);
  result.Target.Z = sin(result.Angle.X) * cos(result.Angle.Y);
  result.TargetLastFrame = result.Target;
  result.Target = result.Position + normalize(result.Target);
  result.Velocity = {};

  return result;
}

static void camera_translate(game_camera& camera, v3 offset)
{
  camera.Position += offset;
  camera.Target += offset;
}

static void camera_set_target(game_camera& camera, v3 target)
{
  camera.Target = target;
}

static void mouse_look(game_camera& camera,
                       float yawDegrees,
                       float pitchDegrees)
{
  camera.Angle += _v2(deg_to_rad(yawDegrees), deg_to_rad(pitchDegrees));

  // clamp look down
  if (camera.Angle.Y < deg_to_rad(-89.99f)) {
    camera.Angle.Y = deg_to_rad(-89.99f);
  }
  // clamp look up
  if (camera.Angle.Y > deg_to_rad(89.99f)) {
    camera.Angle.Y = deg_to_rad(89.99f);
  }

  camera.Target.X = cos(camera.Angle.X) * cos(camera.Angle.Y);
  camera.Target.Y = sin(camera.Angle.Y);
  camera.Target.Z = sin(camera.Angle.X) * cos(camera.Angle.Y);
  camera.Target = camera.Position + normalize(camera.Target);
}

static v3 get_camera_forward(const game_camera& camera)
{
  return normalize(camera.Target - camera.Position);
}

void update_camera_velocity(const uint32_t stateFlags,
                            const game_input_code code,
                            game_camera& camera)
{
  if (!(stateFlags & INPUT_STATE_CHANGED)) {
    return;
  }

  bool keyDown = stateFlags & INPUT_STATE_IS_DOWN;
  float MOVE_SPEED = 15.0f;

  switch (code) {
    case INPUT_CODE_UP:
      camera.Velocity.Z += keyDown ? MOVE_SPEED : -MOVE_SPEED;
      break;
    case INPUT_CODE_DOWN:
      camera.Velocity.Z -= keyDown ? MOVE_SPEED : -MOVE_SPEED;
      break;

    case INPUT_CODE_LEFT:
      camera.Velocity.X -= keyDown ? MOVE_SPEED : -MOVE_SPEED;
      break;
    case INPUT_CODE_RIGHT:
      camera.Velocity.X += keyDown ? MOVE_SPEED : -MOVE_SPEED;
      break;

    default:
      break;
  }

  camera.Velocity.Z = clamp(camera.Velocity.Z, -MOVE_SPEED, MOVE_SPEED);
  camera.Velocity.X = clamp(camera.Velocity.X, -MOVE_SPEED, MOVE_SPEED);
}

void camera_tick(game_camera& camera, const float deltaTime)
{
  v3 forward = get_camera_forward(camera) * camera.Velocity.Z;
  v3 normCross = normalize(cross(get_camera_forward(camera), VECTOR_UP));
  v3 sideways = normCross * camera.Velocity.X;
  v3 translate = forward + sideways;

  camera.Position += translate * deltaTime;
  camera.Target += translate * deltaTime;
}

void move_camera_around_target(const uint32_t stateFlags,
                               const game_input_code code,
                               game_camera& camera,
                               const float deltaTime)
{
  bool keyDown = stateFlags & INPUT_STATE_IS_DOWN;
  if (!keyDown) {
    return;
  }

  v3 forwardDir = get_camera_forward(camera);

  v3 newPosition = {};
  switch (code) {
    case INPUT_CODE_UP:
      newPosition = forwardDir;
      break;
    case INPUT_CODE_DOWN:
      newPosition = -forwardDir;
      break;

    case INPUT_CODE_RIGHT:
      newPosition = cross(forwardDir, VECTOR_UP);
      break;
    case INPUT_CODE_LEFT:
      newPosition = -cross(forwardDir, VECTOR_UP);

      break;

    default:
      break;
  }

  camera.Position += newPosition * deltaTime;
}

void camera_smooth_follow(game_camera& camera)
{
  v3 dir = normalize(camera.TargetLastFrame - camera.Target);

  float currentDistance = length(camera.Target - camera.Position);
  float remainder = (camera.FollowDistance - currentDistance);

  v3 delta = dir * remainder;
  camera.Position += delta;
  camera.TargetLastFrame = camera.Target;
}

void camera_spin_around(game_camera& camera, const double t)
{
  camera.Position.X = (float)sin(t * 0.225f) * 7.0f;
  camera.Position.Z = (float)cos(t * 0.225f) * 22.0f;
}

void camera_update_entity(game_camera& camera)
{
  if (camera.TargetEntity != nullptr) {
    v3 newTarget = camera.TargetEntity->Position;
    float currentRot = atan2(camera.Position.X - camera.Target.X,
                             camera.Position.Z - camera.Target.Z);
    float rotDelta = currentRot - atan2(camera.Position.X - newTarget.X,
                                        camera.Position.Z - newTarget.Z);

    v3 a = camera.Target;
    v3 b = camera.TargetEntity->Position;
    camera.Target = a * (0.9f) + b * 0.1f;
  }
}