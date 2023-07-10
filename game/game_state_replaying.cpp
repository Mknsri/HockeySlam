#include "game_state.h"

STATE_ACTION(puck_set_replayed_position)
{
  game_entity& puck = state.Map.Entities.Puck;
  state.ReplayPuckPos = puck.Body->State.Position;
  for (uint32_t i = 0; i < state.ReplayCommands.Count; i++) {
    game_state_command& c = state.ReplayCommands.Commands[i];
    bool sameType =
      c.Type == game_state_command_type::COMMAND_PUCK_SET_POSITION;
    if (sameType && c.Delay > cmd.Delay) {
      state.ReplayPuckNextPos = c.Position;
      state.ReplayPuckStartTime = state.SimTime;
      state.ReplayPuckTime = state.SimTime + (c.Delay - cmd.Delay);
      break;
    }
  }
}

STATE_ACTION(puck_set_replayed_velocity)
{
  game_entity& puck = state.Map.Entities.Puck;
  PhysicsSystem::set_velocity(puck.Body, cmd.Position);
}

STATE_ACTION(offense_replayed_shoot)
{
  state.Skate = nullptr;
  state.SkateHard = nullptr;
  state.TimeScale = 0.25f;

  offense_shoot_animation(state, cmd.Coords.X);
  state.Map.Entities.Puck.Body->State.Position =
    offense_puck_shoot_offset(state);
}

STATE_ACTION(replay_no_more_data)
{
  state.Map.Entities.Puck.Body->PreviousState.Position = state.ReplayPuckPos;
  state.Map.Entities.Puck.Body->State.Position =
    state.Map.Entities.Puck.Position;
  state.TimeScale = 1.0f;
  state.Map.Entities.Puck.Body->Flags = 0;
  state.ReplayPuckTime = 0.0f;
}

STATE_ACTION(replay_finished)
{
  if (state.SimTime - state.ReadyTime < 1.5f) {
    return;
  }
  state.TimeScale = 1.0f;
  state.ReplayPuckTime = 0.0f;
  state.ReplayCommands.Count = 0;
  if (state.GamesPlayed == 3 || state.Goals == 2 ||
      (state.GamesPlayed == 2 && state.Goals == 0)) {
    state.NextPhase = game_phase::RESULT;
    push_state_command(state.StateCommands, COMMAND_RESTART_MAP);
    push_state_command(state.StateCommands, COMMAND_SHOW_GAME_RESULT);
  } else {
    state.NextPhase = game_phase::SETUP;
    push_state_command(state.StateCommands, COMMAND_RESTART_MAP);
  }
}

STATE_ACTION(offense_replayed_input_update)
{
  state.AimPosition = cmd.Coords;

  v3 offenseVel = tick_offense_velocity(state, cmd.Coords);
  PhysicsSystem::set_velocity(state.Map.Entities.Offense.Body, offenseVel);

  offense_update_charge_animation(state);
  puck_follow_offense(state, offenseVel);
}

STATE_ACTION(start_game_replayed)
{
  offense_setup_skating_animations(state);
}

STATE_ACTION(goalie_react_replayed)
{
  react_puck_shot(state, state.AIGoalie, cmd.Position);
}

STATE_ACTION(offense_setup_begin_skating_animation_replayd)
{
  game_entity& offense = state.Map.Entities.Offense;
  size_t animationCount = offense.Model->AnimationCount;
  animation* animations = offense.Model->Animations;
  AnimationSystem::run_id idleToSkateId = setup_animation(
    "Idle to Skate", animations, animationCount, state.Animator);
  animation_run* idleToSkate = get_run(idleToSkateId, state.Animator);
  idleToSkate->LoopStyle = AnimationSystem::animation_loop_style::NO_LOOP;

  set_animation_for_entity(offense, idleToSkateId, state.Animator);
}

static void setup_replaying_phase(game_state& state)
{
  game_phase phase = game_phase::REPLAYING;
  hook_action(
    state.ReplayHooks, phase, COMMAND_OFFENSE_SHOOT, offense_replayed_shoot);
  hook_action(state.ReplayHooks, phase, COMMAND_GOALIE_MOVE, goalie_update_pos);
  hook_action(state.ReplayHooks,
              phase,
              COMMAND_OFFENSE_PREPARE_TO_SKATE,
              offense_setup_begin_skating_animation_replayd);
  hook_action(state.ReplayHooks,
              phase,
              COMMAND_OFFENSE_START_SKATING,
              start_game_replayed);
  hook_action(state.ReplayHooks,
              phase,
              COMMAND_PUCK_SET_POSITION,
              puck_set_replayed_position);
  hook_action(state.ReplayHooks,
              phase,
              COMMAND_PUCK_SET_VELOCITY,
              puck_set_replayed_velocity);
  hook_action(
    state.ReplayHooks, phase, COMMAND_GOALIE_REACT, goalie_react_replayed);
  hook_action(state.ReplayHooks,
              phase,
              COMMAND_UPDATE_AIM_POSITION,
              offense_replayed_input_update);
  hook_action(state.ReplayHooks,
              game_phase::REPLAYING,
              COMMAND_REPLAY_NO_MORE_DATA,
              replay_no_more_data);
  hook_action(state.InputHooks,
              game_phase::REPLAYING,
              COMMAND_PRIMARY_INPUT_END,
              replay_finished);
}

static void tick_replaying_phase(game_state& state,
                                 game_memory& gameMemory,
                                 render_context& renderContext)
{
  process_state_commands(state.ReplayCommands, state.ReplayHooks, state);

  tick_goalie(state);
  if (state.ReplayPuckTime > 0.0f) {
    state.Map.Entities.Puck.Body->Flags =
      PhysicsSystem::body_flags::PHYSICS_BODY_FLAG_TRIGGER |
      PhysicsSystem::body_flags::PHYSICS_BODY_FLAG_ENTITY_CONTROLLED;
    double startSim = state.SimTime - state.ReplayPuckStartTime;
    double stopSim = state.ReplayPuckTime - state.ReplayPuckStartTime;
    state.ReplayPuckDelta = startSim / stopSim;
    float d = clamp((float)state.ReplayPuckDelta, 0.0f, 0.999f);
    state.Map.Entities.Puck.Position =
      lerp(state.ReplayPuckPos, state.ReplayPuckNextPos, d);
    state.Map.Entities.Puck.Body->PreviousState.Position = state.ReplayPuckPos;
    state.Map.Entities.Puck.Body->State.Position =
      state.Map.Entities.Puck.Position;
  }
}