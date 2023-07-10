#include "game_state.h"

using namespace UISystem;

static void setup_perftest_phase(game_state& state)
{
  state.perfTestPhase = 0;
  game_phase phase = game_phase::PERF_TEST;
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
              puck_set_shoot_position);
  hook_action(
    state.ReplayHooks, phase, COMMAND_PUCK_SET_VELOCITY, puck_set_velocity);
  hook_action(
    state.ReplayHooks, phase, COMMAND_GOALIE_REACT, goalie_react_replayed);
  hook_action(state.ReplayHooks,
              phase,
              COMMAND_UPDATE_AIM_POSITION,
              offense_replayed_input_update);
}

void do_perftest_ui(game_state& state,
                    ui_context* context,
                    render_context& renderContext)
{
  do_sprite(context, &state.Assets->LogoTexture, _v2(0.01f), _v2(0.45f));

  if (state.perfTestPhase < 3) {
    do_text(_v2(0.25f, 0.25f),
            22.0f,
            context,
            state.Assets->TestFont,
            "Testataan suorituskykyä...");
  } else {
    float lowQualityFps = 0.0f;
    float highQualityFps = 0.0f;
    const float dividend = 1.0f / ARRAY_SIZE(state.perfTestFpsLowQuality);
    for (int i = 0; i < ARRAY_SIZE(state.perfTestFpsLowQuality); i++) {
      lowQualityFps += state.perfTestFpsLowQuality[i] * dividend;
      highQualityFps += state.perfTestFpsHighQuality[i] * dividend;
    }

    do_text(_v2(0.45f, 0.27f),
            22.0f,
            context,
            state.Assets->TestFont,
            "Low quality FPS %.2f\nHigh quality FPS %.2f",
            lowQualityFps,
            highQualityFps);

    do_text(_v2(0.25f, 0.40f),
            26.0f,
            context,
            state.Assets->TestFont,
            "Voitko ottaa ruutukaappauksen tästä ja\nlähettää sen Markukselle\n"
            "Kiitos!");
  }

  do_sprite(
    context, &state.Assets->CursorTexture, context->TouchPosition, _v2(0.025f));
}

static void tick_perf_test_phase(game_state& state,
                                 game_memory& gameMemory,
                                 render_context& renderContext)
{

  if (state.RealTime < 1.0f) {
    state.perfTestPhase = 0;
  } else if (state.RealTime < 6.0f) {
    state.perfTestPhase = 1;
  } else if (state.RealTime < 11.0f) {
    state.perfTestPhase = 2;
  } else {
    state.perfTestPhase = 3;
  }

  if (state.perfTestPhase == 2) {
    renderContext.HighQuality = true;
  }

  float fps = 1.0f / state.FrameDelta;
  if (state.perfTestPhase == 1) {
    int i = state.perfTestIndex++ % ARRAY_SIZE(state.perfTestFpsLowQuality);
    state.perfTestFpsLowQuality[i] = fps;
  } else if (state.perfTestPhase == 2) {
    int i = state.perfTestIndex++ % ARRAY_SIZE(state.perfTestFpsLowQuality);
    state.perfTestFpsHighQuality[i] = fps;
  }
  camera_spin_around(state.Map.GameCamera, state.RealTime);

  state.Map.Lights[1].Color = COLOR_CYAN;
  state.Map.Lights[2].Color = COLOR_GREEN;
  state.Map.Lights[3].Color = COLOR_PINK;
  state.Map.Lights[4].Color = COLOR_BLUE;
  for (int j = 2; j < ARRAY_SIZE(state.Map.Lights); j++) {
    state.Map.Lights[j].Vector.X =
      (float)std::sin(state.RealTime * j) * (float)(j * j) + j;
    state.Map.Lights[j].Vector.Y =
      10.0f + (float)std::cos(state.RealTime * j) * (float)(j);
    state.Map.Lights[j].Vector.Z = (float)std::cos(state.RealTime + j) * 15.0f;
  }

  process_state_commands(state.ReplayCommands, state.ReplayHooks, state);
  do_perftest_ui(state, &state.UIContext, renderContext);
  tick_goalie(state);
}