
void tick_goalie(game_state& state)
{
  game_entity& goalie = state.Map.Entities.Goalie;
  game_entity& puck = state.Map.Entities.Puck;
  AnimationSystem::animation_group group =
    AISystem::get_animations_from_ai(state.AIGoalie, goalie);

  AnimationSystem::set_animation_group_for_entity(
    goalie, group, state.Animator);

  v3 target = state.Phase == game_phase::STARTED
                ? state.Map.Entities.Puck.Position
                : state.Map.Entities.Offense.Position;
  AISystem::ai_process_state_change(state.AIGoalie, target, state.SimDelta);
}

STATE_ACTION(goalie_update_pos)
{
  game_entity& goalie = state.Map.Entities.Goalie;
  v3 targetPos = state.Map.Entities.Puck.Position;

  goalie.Body->State.Position = cmd.Position;
  float angleDelta = AISystem::get_goalie_rotation(state.AIGoalie, targetPos);
  float newAngle = normalize_deg(state.AIGoalie.Angle - angleDelta);
  state.AIGoalie.Angle = newAngle;
  goalie.Rotation = quat_from_euler(0.0f, 180.0f + newAngle, 0.0f);
}

STATE_ACTION(goalie_react)
{
  react_puck_shot(state, state.AIGoalie, cmd.Position);
}