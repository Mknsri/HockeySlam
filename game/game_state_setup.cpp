
STATE_ACTION(ready)
{
#if 0
  state.NextPhase = game_phase::RESULT;
  push_state_command(state.StateCommands, COMMAND_SHOW_GAME_RESULT);
#else
  state.NextPhase = game_phase::READY;
#endif
}

STATE_ACTION(restart_map)
{
  state.GoalTime = 0.0f;
  PhysicsSystem::reset_space(state.PhysicsSpace);
  MapSystem::load_map(state, &state.Map);
  state.Map.Entities.Arrow.Scale = _v3(0.0f);

  AnimationSystem::reset_animator(state.Animator);

  AISystem::reset(state.AIGoalie,
                  state.Map.Entities.Goalie.Position,
                  state.Map.Entities.Puck.Position);

  game_entity& offense = state.Map.Entities.Offense;

  AnimationSystem::run_id idleId =
    setup_animation("Idle",
                    offense.Model->Animations,
                    offense.Model->AnimationCount,
                    state.Animator);
  set_animation_for_entity(offense, idleId, state.Animator);

  game_entity& crowdEnd = state.Map.InstancedEntities.CrowdEnd;
  game_entity& crowLSide = state.Map.InstancedEntities.CrowdLSide;
  game_entity& crowRSide = state.Map.InstancedEntities.CrowdRSide;
  AnimationSystem::run_id cheerId =
    setup_animation("cheer",
                    crowdEnd.Model->Animations,
                    crowdEnd.Model->AnimationCount,
                    state.Animator);
  AnimationSystem::set_animation_for_entity(crowdEnd, cheerId, state.Animator);
  AnimationSystem::set_animation_for_entity(crowLSide, cheerId, state.Animator);
  AnimationSystem::set_animation_for_entity(crowRSide, cheerId, state.Animator);
  state.Cheer = AnimationSystem::get_run(cheerId, state.Animator);

  // Reset aimpositionbuffer to something reasonable
  for (size_t i = 0; i < ARRAY_SIZE(state.AimPositionBuffer); i++) {
    state.AimPositionBuffer[i] = _v2(0.0f, -0.1f * i);
  }
}

STATE_ACTION(update_ui_input_start);
STATE_ACTION(update_ui_input);
STATE_ACTION(update_ui_input_end);

STATE_ACTION(reset_game)
{
  restart_map(state, cmd);
  state.PhaseDelay = 0.0f;
  state.GamesPlayed = 0;
  state.Phase = game_phase::SETUP;
  state.NextPhase = game_phase::SETUP;
  state.StateCommands.Count = 0;
  state.ReplayCommands.Count = 0;
  state.StateHooks.Count = 0;
  state.ReplayHooks.Count = 0;
#if HOKI_DEV
  state.DebugHooks.Count = 0;
#endif
  state.InputHooks.Count = 0;
  state.TimeScale = 1.0f;

  state.Goals = 0;

  hook_action(
    state.StateHooks, game_phase::NONE, COMMAND_RESTART_MAP, restart_map);
  hook_action(
    state.StateHooks, game_phase::NONE, COMMAND_RESET_GAME, reset_game);

  setup_started_phase(state);
  setup_ending_phase(state);
  setup_ended_phase(state);
  setup_ready_phase(state);
  setup_result_phase(state);
  setup_replaying_phase(state);

  hook_action(state.InputHooks,
              game_phase::NONE,
              COMMAND_PRIMARY_INPUT_START,
              update_ui_input_start);
  hook_action(
    state.InputHooks, game_phase::NONE, COMMAND_PRIMARY_INPUT, update_ui_input);
  hook_action(state.InputHooks,
              game_phase::NONE,
              COMMAND_PRIMARY_INPUT_END,
              update_ui_input_end);

#if HOKI_DEV
  hook_action(state.DebugHooks,
              game_phase::NONE,
              DEBUG_COMMAND_TOGGLE_MOUSEPICKER,
              toggle_mousepicker);

  hook_action(
    state.DebugHooks, game_phase::NONE, COMMAND_SET_WIREFRAME, set_wireframe);
  hook_action(
    state.DebugHooks, game_phase::NONE, COMMAND_BUILD_SHADERS, build_shaders);
  hook_action(state.DebugHooks,
              game_phase::NONE,
              DEBUG_COMMAND_TOGGLE_DEBUG_CAMERA,
              toggle_debug_camera);

  hook_action(
    state.DebugHooks, game_phase::NONE, COMMAND_MOVE_CAMERA, debug_move_camera);
  hook_action(state.DebugHooks,
              game_phase::NONE,
              DEBUG_COMMAND_TOGGLE_DEBUG_UI,
              debug_toggle_debug_ui);

  state.ShowDebugUI = true;
#endif
  hook_action(
    state.InputHooks, game_phase::SETUP, COMMAND_PRIMARY_INPUT_END, ready);
}

static void tick_setup_phase(game_state& state,
                             game_memory& gameMemory,
                             render_context& renderContext)
{
  camera_smooth_follow(state.Map.GameCamera);

  UISystem::do_sprite(&state.UIContext,
                      &state.Assets->TutorialTexture,
                      _v2(0.35f, 0.05f),
                      _v2(0.75f),
                      0.0f);
#if 0 
  state.NextPhase = game_phase::PERF_TEST;
#else
#endif
  tick_goalie(state);
}