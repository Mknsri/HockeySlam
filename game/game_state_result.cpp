#include "game_state.h"

STATE_ACTION(new_game)
{
  push_state_command(state.StateCommands, COMMAND_RESET_GAME);
}

STATE_ACTION(result_started)
{

  state.Won = state.Goals > 1;
  state.Map.Entities.Puck.Body->Flags =
    PhysicsSystem::body_flags::PHYSICS_BODY_FLAG_STATIC;
  state.Map.Entities.Puck.Body->State.Position = _v3(-1.0f);

  if (!state.Won) {
    game_entity& goalie = state.Map.Entities.Goalie;
    goalie.Position = _v3(0.0f, 0.0f, -16.0f);
    goalie.Rotation = IDENTITY_ROTATION;
    AnimationSystem::run_id celebrateId =
      setup_animation("Celebrate",
                      goalie.Model->Animations,
                      goalie.Model->AnimationCount,
                      state.Animator);
    AnimationSystem::set_animation_for_entity(
      goalie, celebrateId, state.Animator);
  } else {
    game_entity& offense = state.Map.Entities.Offense;
    offense.Position = _v3(6.0f, 0.0f, 13.0f);
    offense.Rotation = quat_from_euler(_v3(0.0f, 180.0f, 0.0f));
    AnimationSystem::run_id celebrateId =
      setup_animation("Celebrate",
                      offense.Model->Animations,
                      offense.Model->AnimationCount,
                      state.Animator);
    AnimationSystem::set_animation_for_entity(
      offense, celebrateId, state.Animator);
  }
}

static void setup_result_phase(game_state& state)
{
  game_phase phase = game_phase::RESULT;
  hook_action(
    state.StateHooks, phase, COMMAND_SHOW_GAME_RESULT, result_started);
}

static void tick_result_phase(game_state& state,
                              game_memory& gameMemory,
                              render_context& renderContext)
{
  if (state.Won) {
    game_entity& offense = state.Map.Entities.Offense;
    do_sprite(&state.UIContext,
              &state.Assets->WinEndTexture,
              _v2(0.15f, 0.05f),
              _v2(0.75));
    state.Map.GameCamera.Position = _v3(2.3f, 1.7f, 13.2f);
    state.Map.GameCamera.Target = offense.Position + _v3(-6.0f, 1.8f, 14.0f);
  } else {
    game_entity& goalie = state.Map.Entities.Goalie;
    do_sprite(&state.UIContext,
              &state.Assets->NoWinEndTexture,
              _v2(0.15f, 0.05f),
              _v2(0.75));
    state.Map.GameCamera.Position = _v3(1.0f, 1.5f, -10.5f);
    state.Map.GameCamera.Target = goalie.Position + _v3(0.0f, 1.5f, 0.0f);
  }
  if (do_button(&state.UIContext,
                _v2(0.2f, 0.6f),
                _v2(0.3f),
                &state.Assets->EndResetTexture)) {
    push_state_command(state.StateCommands, COMMAND_RESET_GAME);
  }
}
