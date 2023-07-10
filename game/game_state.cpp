#include "game_state.h"

#include "game_kinematics.h"
#include "game_offense.cpp"
#include "game_goalie.cpp"

#if 0
#include "launcher2.cpp"
#include "launcher3.cpp"
#include "launcher4.cpp"
#else
#include "launcher5.cpp"
#endif

static void process_state_commands(game_state_command_buffer& buffer,
                                   game_state_hooks& hooks,
                                   game_state& state);

#include "game_state_debug.cpp"
#include "game_state_ready.cpp"
#include "game_state_started.cpp"
#include "game_state_ending.cpp"
#include "game_state_ended.cpp"
#include "game_state_replaying.cpp"
#include "game_state_perf_test.cpp"
#include "game_state_result.cpp"
#include "game_state_setup.cpp"

using AnimationSystem::add_animation_for_entity;
using AnimationSystem::ANIMATION_LOOPS_INFINITE;
using AnimationSystem::animation_run;
using AnimationSystem::setup_animation;

static void hook_action(game_state_hooks& hooks,
                        game_phase phase,
                        const game_state_command_type type,
                        state_action_func action)
{
  HOKI_ASSERT(hooks.Count < ARRAY_SIZE(hooks.Hooks));
  game_state_hook hook = {};
  hook.Action = action;
  hook.Type = type;
  hook.Phase = phase;

  hooks.Hooks[hooks.Count++] = hook;
}

STATE_ACTION(store_command_to_replay_buffer)
{
  double delay = state.SimTime - state.ReadyTime;
  push_state_command(state.ReplayCommands, cmd.Type, cmd.Position, delay);
}

static void process_state_commands(game_state_command_buffer& buffer,
                                   game_state_hooks& hooks,
                                   game_state& state)
{
  float offsetFromStart = (float)(state.SimTime - state.ReadyTime);
  for (size_t i = 0; i < buffer.Count; i++) {
    game_state_command& command = buffer.Commands[i];

    bool hookActivated = false;
    for (size_t j = 0; j < hooks.Count; j++) {
      if (command.Processed) {
        break;
      }
      game_state_hook& hook = hooks.Hooks[j];

      bool sameType = command.Type == hook.Type;
      bool phaseMatches =
        hook.Phase == state.Phase || hook.Phase == game_phase::NONE;
      bool instant = command.Delay == 0.0f;
      bool pastDelay = (command.Delay + state.SimDelta) < offsetFromStart;
      if (sameType && (instant || pastDelay) && phaseMatches) {
        hookActivated = true;
        hook.Action(state, command);
      }
    }

    // Mark as processed if at least 1 hook activated
    if (!command.Processed) {
      command.Processed = hookActivated;
    }
  }
}

STATE_ACTION(update_ui_input_start)
{
  if (cmd.Coords.X != 0.0f && cmd.Coords.Y != 0.0f) {
    UISystem::update_input_pos_absolute(&state.UIContext, cmd.Coords);
  }
  UISystem::update_input_went_down(&state.UIContext, true);
}

STATE_ACTION(update_ui_input)
{
  UISystem::update_input_pos_delta(&state.UIContext, cmd.Coords);

#if HOKI_DEV
  if (state.DebugCameraActive) {
    float x = cmd.Coords.X;
    float y = cmd.Coords.Y;
    mouse_look(state.DebugCamera, x * 10.0f, -y * 10.0f);
  }
#endif
}

STATE_ACTION(update_ui_input_end)
{
  UISystem::update_input_went_down(&state.UIContext, false);
}

static void initialize_state(game_memory& memory,
                             game_state& state,
                             const render_context& context,
                             Asset::game_assets* assets)
{
  state.Hz = 256;
  state.ToneVolume = 10550;
  state.TSine = 0;

  state.TimeScale = 1.0f;

  memory.Initialized = true;

  state.GamesPlayed = 0;
  state.Goals = 0;
  state.StateCommands = {};
  state.ReplayCommands = {};

  state.Assets = assets;
  // create_entity(&assets->StandsModel, &state.Map.Entities.Stands);
  create_entity("Field", &assets->FieldModel, state.Map.Entities.Field);
  create_entity("Posts", &assets->PostsModel, state.Map.Entities.Posts);
  create_entity("Offense", &assets->OffenseModel, state.Map.Entities.Offense);
  create_entity("Puck", &assets->PuckModel, state.Map.Entities.Puck);
  create_entity("Goalie", &assets->GoalieModel, state.Map.Entities.Goalie);
  create_entity(
    "SeatEnd", &assets->SeatModel, state.Map.InstancedEntities.SeatEnd);
  create_entity(
    "SeatL", &assets->SeatModel, state.Map.InstancedEntities.SeatLSide);
  create_entity(
    "SeatR", &assets->SeatModel, state.Map.InstancedEntities.SeatRSide);
  create_entity(
    "CrowdL", &assets->CrowdModel, state.Map.InstancedEntities.CrowdLSide);
  create_entity(
    "CrowdR", &assets->CrowdModel, state.Map.InstancedEntities.CrowdRSide);
  create_entity(
    "CrowdEnd", &assets->CrowdModel, state.Map.InstancedEntities.CrowdEnd);
  create_entity("Arrow", &assets->ArrowModel, state.Map.Entities.Arrow);

  state.UIContext = UISystem::create_context(100, state.Assets);

  state.PhysicsSpace = PhysicsSystem::create_space();

  // RESET_GAME unhooks everything so hook this here, ew
  hook_action(
    state.StateHooks, game_phase::NONE, COMMAND_RESET_GAME, reset_game);

  push_state_command(state.StateCommands, COMMAND_RESET_GAME);
}

static void phase_switched(game_state& state,
                           game_phase newPhase,
                           game_phase oldPhase,
                           game_memory& memory)
{
  switch (oldPhase) {
    case game_phase::ENDED:
      break;

    default:
      break;
  }

  switch (newPhase) {
    case game_phase::SETUP:
      state.ReplayCommands.Count = 0;
      break;

    case game_phase::READY:
      state.ReadyTime = state.SimTime;
      break;

    case game_phase::REPLAYING:
      state.ReadyTime = state.SimTime;
      push_state_command(state.StateCommands, COMMAND_RESTART_MAP);
#if 0
      memory.WriteFile(
        "replay.rip", &state.ReplayCommands, sizeof(state.ReplayCommands));
#endif
      break;
    case game_phase::PERF_TEST:
      setup_perftest_phase(state);
      state.TimeScale = 0.3f;
      memory.ReadFile("/replay.rip", (uint8_t*)&state.ReplayCommands);
      break;

    default:
      break;
  }
}

static void tick_state(game_state& state,
                       game_memory& gameMemory,
                       render_context& renderContext)
{
#if HOKI_DEV
  if (state.Picked != nullptr) {
    render_command& outlineRenderCommand =
      renderContext.Commands.Commands[renderContext.Commands.Count++];
    outlineRenderCommand.Type = RENDER_COMMAND_TYPE_OUTLINE;
    outlineRenderCommand.Entity = state.Picked;
  }
#endif

  process_state_commands(state.StateCommands, state.InputHooks, state);

  process_state_commands(state.StateCommands, state.StateHooks, state);

  switch (state.Phase) {
    case game_phase::SETUP:
      tick_setup_phase(state, gameMemory, renderContext);
      break;

    case game_phase::READY:
      tick_ready_phase(state, gameMemory, renderContext);
      break;

    case game_phase::STARTED:
      tick_started_phase(state, gameMemory, renderContext);
      break;

    case game_phase::ENDING:
      tick_ending_phase(state, gameMemory, renderContext);
      break;

    case game_phase::ENDED:
      tick_ended_phase(state, gameMemory, renderContext);
      break;

    case game_phase::REPLAYING:
      tick_replaying_phase(state, gameMemory, renderContext);
      break;

    case game_phase::PERF_TEST:
      tick_perf_test_phase(state, gameMemory, renderContext);
      break;

    case game_phase::RESULT:
      tick_result_phase(state, gameMemory, renderContext);
      break;

    default:
      break;
  }

  if (state.Phase != state.NextPhase) {
    if (state.PhaseDelay < state.SimDelta) {
      game_phase old = state.Phase;
      state.Phase = state.NextPhase;
      phase_switched(state, state.Phase, old, gameMemory);
      state.PhaseDelay = 0.0f;
    } else {
      state.PhaseDelay -= state.SimDelta;
    }
  }

#if HOKI_DEV
  process_state_commands(state.StateCommands, state.DebugHooks, state);
#endif
#if HOKI_DEV
  if (state.DebugCameraActive) {
    camera_tick(state.DebugCamera, state.FrameDelta);
  }
#endif
  camera_tick(state.Map.GameCamera, state.FrameDelta);

  v3 cameraForward = get_camera_forward(state.Map.GameCamera);
  if (state.Phase == game_phase::REPLAYING) {
    cameraForward = get_camera_forward(state.Map.ReplayCamera);
  }
  MapSystem::instanced_entity_list& iList = state.Map.InstancedEntities;
  if (cameraForward.Z > 0.0f) {
    iList.SeatEnd.Position.X = 12.0f;
    iList.SeatEnd.Position.Z = 67.0f;
    iList.SeatEnd.Rotation = quat_from_euler(_v3(0.0f, 180.0f, 0.0f));
    iList.CrowdEnd.Position.X = iList.SeatEnd.Position.X;
    iList.CrowdEnd.Position.Z = 66.0f;
    iList.CrowdEnd.Rotation = iList.SeatEnd.Rotation;
  } else {
    iList.SeatEnd.Position.X = -12.0f;
    iList.SeatEnd.Position.Z = -38.0f;
    iList.SeatEnd.Rotation = quat_from_euler(_v3(0.0f, 0.0f, 0.0f));
    iList.CrowdEnd.Position.X = iList.SeatEnd.Position.X;
    iList.CrowdEnd.Position.Z = -36.0f;
    iList.CrowdEnd.Rotation = iList.SeatEnd.Rotation;
  }

  push_render_instanced(renderContext, &iList.SeatEnd);
  push_render_instanced(renderContext, &iList.SeatLSide);
  push_render_instanced(renderContext, &iList.SeatRSide);
  push_render_instanced(renderContext, &iList.CrowdEnd);
  push_render_instanced(renderContext, &iList.CrowdLSide);
  push_render_instanced(renderContext, &iList.CrowdRSide);
}
