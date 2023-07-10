#include "game_state.h"

STATE_ACTION(joystick_activated)
{
  push_state_command(state.StateCommands, COMMAND_OFFENSE_PREPARE_TO_SKATE);
}

STATE_ACTION(start_game)
{
  offense_setup_skating_animations(state);
  state.NextPhase = game_phase::STARTED;
}

static void setup_ready_phase(game_state& state)
{
  game_phase phase = game_phase::READY;
  hook_action(
    state.StateHooks, phase, COMMAND_ACTIVATE_JOYSTICK, joystick_activated);
  hook_action(state.StateHooks,
              phase,
              COMMAND_ACTIVATE_JOYSTICK,
              store_command_to_replay_buffer);
  hook_action(state.StateHooks,
              phase,
              COMMAND_OFFENSE_PREPARE_TO_SKATE,
              offense_setup_begin_skating_animation);
  hook_action(state.StateHooks,
              phase,
              COMMAND_OFFENSE_PREPARE_TO_SKATE,
              store_command_to_replay_buffer);
  hook_action(state.StateHooks,
              phase,
              COMMAND_OFFENSE_START_SKATING,
              store_command_to_replay_buffer);
  hook_action(
    state.StateHooks, phase, COMMAND_OFFENSE_START_SKATING, start_game);
}

static void tick_ready_phase(game_state& state,
                             game_memory& gameMemory,
                             render_context& renderContext)
{
  camera_smooth_follow(state.Map.GameCamera);
  tick_goalie(state);
}
