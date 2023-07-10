using AnimationSystem::add_animation_for_entity;
using AnimationSystem::setup_animation;

STATE_ACTION(puck_set_velocity)
{
  game_entity& puck = state.Map.Entities.Puck;
  PhysicsSystem::set_velocity(puck.Body, cmd.Position);
}

STATE_ACTION(offense_setup_begin_skating_animation)
{
  game_entity& offense = state.Map.Entities.Offense;
  size_t animationCount = offense.Model->AnimationCount;
  animation* animations = offense.Model->Animations;
  AnimationSystem::run_id idleToSkateId = setup_animation(
    "Idle to Skate", animations, animationCount, state.Animator);
  animation_run* idleToSkate = get_run(idleToSkateId, state.Animator);
  idleToSkate->LoopStyle = AnimationSystem::animation_loop_style::NO_LOOP;

  set_animation_for_entity(offense, idleToSkateId, state.Animator);

  double delay =
    (state.SimTime - state.ReadyTime) + idleToSkate->Animation->Duration;
  push_state_command(state.StateCommands, COMMAND_OFFENSE_START_SKATING, delay);
}

void offense_setup_skating_animations(game_state& state)
{
  game_entity& offense = state.Map.Entities.Offense;
  size_t animationCount = state.Map.Entities.Offense.Model->AnimationCount;
  animation* animations = state.Map.Entities.Offense.Model->Animations;
  AnimationSystem::run_id skateId =
    setup_animation("Slide", animations, animationCount, state.Animator);

  set_animation_for_entity(offense, skateId, state.Animator);
  state.Skate = AnimationSystem::get_run(skateId, state.Animator);
  state.Skate->Weight = 0.0f;

  AnimationSystem::run_id skateHardId =
    setup_animation("Skate", animations, animationCount, state.Animator);

  add_animation_for_entity(offense, skateHardId, state.Animator);
  state.SkateHard = AnimationSystem::get_run(skateHardId, state.Animator);

  AnimationSystem::run_id chargeUpId =
    setup_animation("ChargeUp", animations, animationCount, state.Animator);
  add_animation_for_entity(offense, chargeUpId, state.Animator);
  state.ChargeUp = AnimationSystem::get_run(chargeUpId, state.Animator);
#if 0 // launcher2
  state.ChargeUp->Driver = AnimationSystem::animation_driver::INPUT;
#endif
  state.ChargeUp->Weight = 0.0f;
  state.ChargeUp->LoopStyle = AnimationSystem::animation_loop_style::NO_LOOP;
}

float offense_shoot_animation(game_state& state, float aimPower)
{
  game_entity& offense = state.Map.Entities.Offense;
  size_t animationCount = state.Map.Entities.Offense.Model->AnimationCount;
  animation* animations = state.Map.Entities.Offense.Model->Animations;

  AnimationSystem::run_id shootId = AnimationSystem::setup_animation(
    "Shoot", animations, animationCount, state.Animator);
  AnimationSystem::set_animation_for_entity(offense, shootId, state.Animator);
  animation_run* shootAnim = AnimationSystem::get_run(shootId, state.Animator);
  shootAnim->LoopStyle = AnimationSystem::animation_loop_style::NO_LOOP;
  shootAnim->CurrentTime =
    shootAnim->Animation->Duration * clamp(0.9f - aimPower, 0.0f, 0.9f);
  shootAnim->Loops = 1;

  AnimationSystem::run_id shootToSlideId = setup_animation(
    "Shoot to Slide", animations, animationCount, state.Animator);
  add_animation_for_entity(offense, shootToSlideId, state.Animator);
  animation_run* shootToSlide =
    AnimationSystem::get_run(shootToSlideId, state.Animator);
  shootToSlide->Loops = 1;
  shootToSlide->StartDelay =
    shootAnim->Animation->Duration - shootAnim->CurrentTime;
  shootToSlide->LoopStyle = AnimationSystem::animation_loop_style::NO_LOOP;

  AnimationSystem::run_id slideId =
    setup_animation("Slide", animations, animationCount, state.Animator);
  add_animation_for_entity(offense, slideId, state.Animator);

  animation_run* slide = AnimationSystem::get_run(slideId, state.Animator);
  slide->StartDelay =
    shootToSlide->StartDelay + shootToSlide->Animation->Duration;

  return shootAnim->Animation->Duration - shootAnim->CurrentTime - 0.2f;
}

void offense_update_charge_animation(game_state& state)
{
  if (!(state.Skate && state.SkateHard)) {
    return;
  }

#if 1 // launcher2 / launcher5
  float aimPower = 0.5f + state.AimPosition.Y;
  state.Skate->Weight = aimPower;
  state.SkateHard->Weight = 1.0f - aimPower;
  state.ChargeUp->Weight = 0.0f;
#else

  if (state.AimPosition.Y < 0.0f) {
    float skateWeight = 0.5f + state.AimPosition.Y;

    state.SkateHard->Weight = 1.0f - skateWeight;
    state.Skate->Weight = skateWeight;
    state.ChargeUp->Weight = 0.0f;
  } else {
    state.SkateHard->Weight = 0.0f;
    state.Skate->Weight = 1.0f - state.AimPower;
    state.ChargeUp->Weight = state.AimPower;
  }
#endif
}

void puck_follow_offense(game_state& state, const v3 offenseVel)
{
  game_entity& offense = state.Map.Entities.Offense;
  game_entity& goal = state.Map.Entities.Posts;
  game_entity& puck = state.Map.Entities.Puck;

  float angleToGoal = atan2(goal.Position.X - offense.Position.X,
                            goal.Position.Z - offense.Position.Z);
  float offenseRotation = rad_to_deg(angleToGoal);
  offense.Rotation = quat_from_euler(0.0f, offenseRotation, 0.0f);

  float aimPower = 0.5f + state.AimPosition.Y;
  // Follows skate/skatehard animation stick positions
  v2 minOffset = _v2(0.2f, 2.0f);
  v2 maxOffset = _v2(-1.6f, 0.2f);
  v2 offset = (1.0f - aimPower) * minOffset + aimPower * maxOffset;
  // Rotate around angleToGoal
  float rotatedX = offset.X * cosf(angleToGoal) - offset.Y * -sinf(angleToGoal);
  float rotatedY = offset.X * -sinf(angleToGoal) + offset.Y * cosf(angleToGoal);
  offset = _v2(rotatedX, rotatedY);

  // puck.Body->PreviousState.Position = puck.Body->State.Position;
  puck.Body->State.Position.X = offense.Body->State.Position.X + offset.X;
  puck.Body->State.Position.Z = offense.Body->State.Position.Z + offset.Y;

  PhysicsSystem::set_velocity(puck.Body, offenseVel);
}

v3 offense_puck_shoot_offset(const game_state& state)
{
  const game_entity& offense = state.Map.Entities.Offense;
  const game_entity& goal = state.Map.Entities.Posts;

  float angleToGoal = atan2(goal.Position.X - offense.Position.X,
                            goal.Position.Z - offense.Position.Z);
  float rotatedX = -1.6f * cosf(angleToGoal) - 0.2f * -sinf(angleToGoal);
  float rotatedY = -1.6f * -sinf(angleToGoal) + 0.2f * cosf(angleToGoal);

  return _v3(offense.Body->State.Position.X + rotatedX,
             offense.Body->State.Position.Y,
             offense.Body->State.Position.Z + rotatedY);
}