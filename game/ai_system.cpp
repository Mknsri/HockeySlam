#include "ai_system.h"
#include "game_kinematics.h"

namespace AISystem {
const float MAX_SIDEWAYS_MOVEMENT = 2.5f;

void reset(ai_goalie& ai, const v3 startPosition, const v3 targetPosition)
{
  ai.State = ai_goalie_state::STATE_NONE;
  ai.NextState = ai_goalie_state::STATE_IDLE;
  ai.LungeDir = ai_goalie_lunge_dir::LUNGE_NONE;
  ai.Position = startPosition;

  v3 goalieToPuck = startPosition - targetPosition;
  ai.Angle = normalize_deg(rad_to_deg(atan2(goalieToPuck.X, goalieToPuck.Z)));
}

void ai_process_state_change(ai_goalie& ai,
                             const v3 target,
                             const float deltaTime)
{
  if (ai.NextState != ai_goalie_state::STATE_NONE) {
    ai.State = ai.NextState;
    ai.NextState = ai_goalie_state::STATE_NONE;
  }

  if (ai.LungeDir == ai_goalie_lunge_dir::LUNGE_NONE &&
      ai.NextState == ai_goalie_state::STATE_NONE) {
    bool targetInFront = ai.Position.Z < target.Z;
    bool targetLeft = targetInFront ? ai.Position.X < (target.X - 0.1f)
                                    : ai.Position.X > (target.X + 0.1f);
    bool targetRight = targetInFront ? ai.Position.X > (target.X + 0.1f)
                                     : ai.Position.X < (target.X - 0.1f);

    if (targetLeft) {
      ai.NextState = ai_goalie_state::STATE_SLIDE_LEFT;
    } else if (targetRight) {
      ai.NextState = ai_goalie_state::STATE_SLIDE_RIGHT;
    } else if (ai.MovementAmount == 0.0f) {
      ai.NextState = ai_goalie_state::STATE_IDLE;
    }

    if (ai.State == ai.NextState) {
      ai.NextState = ai_goalie_state::STATE_NONE;
    }
  }
}

v3 get_goalie_next_pos(ai_goalie& ai, const v3 target)
{
  if (ai.LungeDir == ai_goalie_lunge_dir::LUNGE_LEFT_LOW ||
      ai.LungeDir == ai_goalie_lunge_dir::LUNGE_MIDDLE_LOW) {
    return ai.Position;
  }

  float maxMoveSpeed = 0.2f;
  float desiredX = target.X * 0.95f;
  v3 currPos = ai.Position;
  float diff = clamp((desiredX - currPos.X), -maxMoveSpeed, maxMoveSpeed);

  float newX = currPos.X + diff;
  DEBUG_LOG("D: %f T.X: %f G.X: %f\n", diff, target.X, ai.Position.X);
  newX = clamp(newX, -MAX_SIDEWAYS_MOVEMENT, MAX_SIDEWAYS_MOVEMENT);

  diff = abs_f(diff);
  if (diff < 0.1f) {
    return currPos;
  } else if (diff > 0.04f) {
    ai.MovementAmount = clamp(diff, 0.8f, 1.0f);
  } else {
    ai.MovementAmount = clamp(diff, 0.6f, 1.0f);
  }

  if (equal_f(abs_f(ai.Position.X), MAX_SIDEWAYS_MOVEMENT)) {
    ai.MovementAmount = 0.0f;
  }

  ai.Position.X = newX;
  return ai.Position;
}

float get_goalie_rotation(ai_goalie& ai, const v3 target)
{
  if (ai.State == AISystem::ai_goalie_state::STATE_LUNGE) {
    return 0.0f;
  }

  v3 goalieToPuck = ai.Position - target;
  float angleDelta =
    ai.Angle - rad_to_deg(atan2(goalieToPuck.X, goalieToPuck.Z));
  angleDelta = normalize_deg(angleDelta);
  if (angleDelta > 20 && ai.MovementAmount < 0.2f) {
    ai.MovementAmount += 0.2f;
  }

  return angleDelta;
}

void react_puck_shot(game_state& state, ai_goalie& ai, const v3 reactDirection)
{
  ai.NextState = AISystem::ai_goalie_state::STATE_LUNGE;

#if HOKI_DEV
  state.DebugAimLine.Position = state.AIGoalie.Position + _v3(0.0f, 1.0f, 0.0f);
  state.DebugAimLine.Length = reactDirection * 2.0f;
#endif

  bool left = reactDirection.X > 0.0f;
  bool low = reactDirection.Y < 0.25f;
  bool high = reactDirection.Y > 0.45f;

  if (left) {
    if (high) {
      ai.LungeDir = ai_goalie_lunge_dir::LUNGE_LEFT_HIGH;
    } else if (low) {
      ai.LungeDir = ai_goalie_lunge_dir::LUNGE_LEFT_LOW;
    } else {
      ai.LungeDir = ai_goalie_lunge_dir::LUNGE_LEFT_MIDDLE;
    }
  } else {
    if (high) {
      ai.LungeDir = ai_goalie_lunge_dir::LUNGE_RIGHT_HIGH;
    } else if (low) {
      ai.LungeDir = ai_goalie_lunge_dir::LUNGE_RIGHT_LOW;
    } else {
      ai.LungeDir = ai_goalie_lunge_dir::LUNGE_RIGHT_MIDDLE;
    }
  }
}

AnimationSystem::animation_group get_animations_from_ai(
  const ai_goalie& ai,
  const game_entity& entity)
{
  AnimationSystem::animation_group result = {};
  result.LoopStyle = AnimationSystem::animation_loop_style::WRAP;
  if (ai.NextState == ai.State) {
    result.UpdateWeightsOnly = true;
  }

  size_t animationCount = entity.Model->AnimationCount;
  animation* animations = entity.Model->Animations;
  switch (ai.NextState) {
    case ai_goalie_state::STATE_IDLE:
      push_animation_to_group(result, "Idle", animations, animationCount, 1.0f);
      break;

    case ai_goalie_state::STATE_SLIDE_LEFT:
      push_animation_to_group(
        result, "Idle", animations, animationCount, 1.0f - ai.MovementAmount);
      push_animation_to_group(
        result, "Left Shuffle", animations, animationCount, ai.MovementAmount);
      break;

    case ai_goalie_state::STATE_SLIDE_RIGHT:
      push_animation_to_group(
        result, "Idle", animations, animationCount, 1.0f - ai.MovementAmount);
      push_animation_to_group(
        result, "Right Shuffle", animations, animationCount, ai.MovementAmount);
      break;

    case ai_goalie_state::STATE_LUNGE: {
      result.LoopStyle = AnimationSystem::animation_loop_style::NO_LOOP;

      switch (ai.LungeDir) {
        case ai_goalie_lunge_dir::LUNGE_LEFT_HIGH:
          push_animation_to_group(
            result, "HighCatchLeft", animations, animationCount, 1.0f);
          break;

        case ai_goalie_lunge_dir::LUNGE_LEFT_MIDDLE:
          push_animation_to_group(
            result, "HighCatchLeft", animations, animationCount, 0.3f);
          push_animation_to_group(
            result, "Left Catch", animations, animationCount, 0.7f);
          break;

        case ai_goalie_lunge_dir::LUNGE_LEFT_LOW:
          push_animation_to_group(
            result, "Left Catch", animations, animationCount, 1.0f);
          break;

        case ai_goalie_lunge_dir::LUNGE_RIGHT_HIGH:
          push_animation_to_group(
            result, "RightBlockHigh", animations, animationCount, 1.0f);
          break;

        case ai_goalie_lunge_dir::LUNGE_RIGHT_MIDDLE:
          push_animation_to_group(
            result, "RightBlock", animations, animationCount, 0.3f);
          push_animation_to_group(
            result, "RightStick", animations, animationCount, 0.7f);
          break;

        case ai_goalie_lunge_dir::LUNGE_RIGHT_LOW:
          push_animation_to_group(
            result, "RightStick", animations, animationCount, 1.0f);
          break;

        default:
          HOKI_ASSERT(false);
      }
    } break;

    case ai_goalie_state::STATE_NONE:
      break;

    default:
      HOKI_ASSERT_MESSAGE(false, "Unknown AI state", 0);
      break;
  }

  return result;
}
}