#include "game_state.h"

STATE_ACTION(start_replay)
{
  if (state.SimTime - state.ReadyTime - state.GoalTime < 3.0f) {
    return;
  }
  state.NextPhase = game_phase::REPLAYING;
  double delay = state.SimTime - state.ReadyTime;
  game_entity& puck = state.Map.Entities.Puck;
  push_state_command(state.ReplayCommands,
                     COMMAND_PUCK_SET_POSITION,
                     puck.Body->State.Position,
                     delay);
  push_state_command(
    state.ReplayCommands, COMMAND_PUCK_SET_VELOCITY, _v3(0.0f), delay);
  push_state_command(state.ReplayCommands, COMMAND_REPLAY_NO_MORE_DATA, delay);
}

static void setup_ended_phase(game_state& state)
{
  game_phase phase = game_phase::ENDED;
  hook_action(state.InputHooks, phase, COMMAND_PRIMARY_INPUT_END, start_replay);
  hook_action(state.StateHooks, phase, COMMAND_GOALIE_MOVE, goalie_update_pos);
}

static void tick_ended_phase(game_state& state,
                             game_memory& gameMemory,
                             render_context& renderContext)
{
  if (state.GoalTime > 0.0f) {
    float textDelay = 1.0f;
    float textDurationIn = 1.5f;
    UISystem::do_animated_sprite(&state.UIContext,
                                 &state.Assets->GoalFlash1Texture,
                                 _v2(3.0f),
                                 _v2(3.0f, 0.5f),
                                 _v2(-3.0f, 0.5f),
                                 state.GoalTime + 0.2f,
                                 3.2f,
                                 state.SimTime - state.ReadyTime,
                                 UISystem::EaseInQuad);
    UISystem::do_animated_sprite(&state.UIContext,
                                 &state.Assets->PuckFlashTexture,
                                 _v2(4.0f, 2.0f),
                                 _v2(-2.0f, 0.5f),
                                 _v2(4.5f, 0.5f),
                                 state.GoalTime,
                                 3.4f,
                                 state.SimTime - state.ReadyTime,
                                 UISystem::EaseOutQuad);

    UISystem::do_scale_animated_sprite(&state.UIContext,
                                       &state.Assets->GoalTextTexture,
                                       _v2(0.5f),
                                       _v2(0.01f),
                                       _v2(1.0f),
                                       state.GoalTime + textDelay,
                                       textDurationIn,
                                       state.SimTime - state.ReadyTime,
                                       UISystem::EaseOutBack);
    UISystem::do_scale_animated_sprite(&state.UIContext,
                                       &state.Assets->GoalTextTexture,
                                       _v2(0.5f),
                                       _v2(1.0f),
                                       _v2(0.01f),
                                       state.GoalTime + textDurationIn +
                                         textDelay,
                                       0.3f,
                                       state.SimTime - state.ReadyTime,
                                       UISystem::EaseInSine);
  } else {
    v3 nextPos = AISystem::get_goalie_next_pos(
      state.AIGoalie, state.Map.Entities.Puck.Position);
    push_state_command(state.StateCommands, COMMAND_GOALIE_MOVE, nextPos, 0.0);
  }
  tick_goalie(state);

  game_entity& puck = state.Map.Entities.Puck;
  static v3 pos = _v3(0.0f);
  if (pos != puck.Body->State.Position) {
    pos = puck.Body->State.Position;
    push_state_command(state.ReplayCommands,
                       COMMAND_PUCK_SET_POSITION,
                       puck.Body->State.Position,
                       state.SimTime - state.ReadyTime);
  }

#if HOKI_DEV
  add_debug_rendercommand(renderContext, &state.DebugAimLine);
#endif
}
