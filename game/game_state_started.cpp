#include "game_state.h"

STATE_ACTION(offense_input_update)
{
  game_entity& offense = state.Map.Entities.Offense;
  game_entity& puck = state.Map.Entities.Puck;
  v3 offenseVel = tick_offense_velocity(state, cmd.Coords);
  PhysicsSystem::set_velocity(offense.Body, offenseVel);

  offense_update_charge_animation(state);
  puck_follow_offense(state, offense.Body->State.Velocity);
}

STATE_ACTION(offense_input_end)
{
  game_entity& puck = state.Map.Entities.Puck;
  float aimPower = get_aim_power(state);
  push_state_command(state.StateCommands, COMMAND_OFFENSE_SHOOT, _v2(aimPower));

  push_state_command(
    state.StateCommands, COMMAND_PUCK_SET_VELOCITY, puck.Body->State.Velocity);

  state.NextPhase = game_phase::ENDING;
}

static void setup_started_phase(game_state& state)
{
  game_phase phase = game_phase::STARTED;

  hook_action(
    state.InputHooks, phase, COMMAND_PRIMARY_INPUT_END, offense_input_end);

  hook_action(state.StateHooks,
              phase,
              COMMAND_UPDATE_AIM_POSITION,
              store_command_to_replay_buffer);
  hook_action(
    state.StateHooks, phase, COMMAND_UPDATE_AIM_POSITION, offense_input_update);

  hook_action(state.StateHooks, phase, COMMAND_GOALIE_MOVE, goalie_update_pos);
  hook_action(state.StateHooks,
              phase,
              COMMAND_GOALIE_MOVE,
              store_command_to_replay_buffer);
}

static void tick_started_phase(game_state& state,
                               game_memory& gameMemory,
                               render_context& renderContext)
{
  game_camera& camera = state.Map.GameCamera;
  game_entity& goalie = state.Map.Entities.Goalie;
  game_entity& offense = state.Map.Entities.Offense;
  game_entity& puck = state.Map.Entities.Puck;
  camera_smooth_follow(camera);
  camera.Target =
    _v3(offense.Position.X * 0.9f, 0.0f, offense.Position.Z - 5.0f);

  bool goalieTouched = offense.Body->CollisionCount > 1;
  bool pastGoalie = state.Map.Entities.Offense.Position.Z <
                    state.Map.Entities.Goalie.Position.Z;
  if (pastGoalie || goalieTouched) {
    // Shoot for the player if past the goal or the goalie was touched
    push_state_command(state.StateCommands, COMMAND_PRIMARY_INPUT_END);
  } else {
    push_state_command(
      state.StateCommands, COMMAND_UPDATE_AIM_POSITION, state.AimPosition);
  }
  tick_goalie(state);
  v3 nextPos = AISystem::get_goalie_next_pos(state.AIGoalie,
                                             state.Map.Entities.Puck.Position);
  push_state_command(state.StateCommands, COMMAND_GOALIE_MOVE, nextPos, 0.0);

#if HOKI_DEV
  add_debug_rendercommand(renderContext, &state.DebugAimLine);
#endif
}