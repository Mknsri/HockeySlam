#include "game_main.h"

#include "game_memory.cpp"
#include "game_collections.cpp"
#include "game_math.h"
#include "game_rand.cpp"

#include "asset/asset_string.cpp"
#include "asset/asset_text.cpp"
#include "asset/asset_file.cpp"
#include "asset/asset_dds.cpp"
#include "asset/asset_texture.cpp"
#include "asset/asset_material.cpp"
#include "asset/asset_model.cpp"
#include "asset/asset_shader.cpp"

#include "game_render.cpp"
#include "game_camera.cpp"
#include "game_entity.cpp"
#include "game_light.cpp"

#include "imui_easing.cpp"
#include "imui_system.cpp"
#include "physics_system.cpp"
#include "anim_system.cpp"
#include "map_system.cpp"
#include "ui_system.cpp"
#include "ai_system.cpp"
#include "snd_system.cpp"

#include "game_assets.cpp"
#include "game_state.cpp"
#include "game_input.cpp"
#include "game_light.h"

extern "C" GAME_MAIN(GameMain)
{
  game_state& state = *((game_state*)gameMemory.PermanentStorage);

  if (!gameMemory.Initialized) {
    init_allocator(gameMemory);
#if HOKI_DEV
    DEBUG_LOG = gameMemory.Log;
#endif
    DEBUG_LOG("Initialized.\n");
    allocate(sizeof(game_state));
    Asset::game_assets* assets =
      (Asset::game_assets*)allocate_t(sizeof(Asset::game_assets));
    *assets = Asset::load_assets(gameMemory);
    initialize_state(gameMemory, state, renderContext, assets);
  }

  if (!renderContext.Initialized) {
    renderContext.RenderableStore = (hash_table*)allocate(sizeof(hash_table));
    *renderContext.RenderableStore = create_hash_table(512);
    Asset::compile_shaders(renderContext, *state.Assets);

    renderContext.HighQuality = true;
    UISystem::reset_context(&state.UIContext, renderContext);
    renderContext.Initialized = true;
    push_render_initialize(renderContext);
  }

#if HOKI_DEV
  if (!memory_pools_loaded()) {
    set_memory_pools(gameMemory);
  }
  HOKI_ASSERT(memory_pools_loaded());
#endif

  state.FrameDelta = deltaTime;
  state.RealTime += deltaTime;
  state.SimDelta = deltaTime * state.TimeScale;
  state.SimTime += deltaTime * state.TimeScale;

  handle_input(inputBuffer, state.StateCommands);

  tick_state(state, gameMemory, renderContext);
  camera_tick(state.Map.GameCamera, state.SimDelta);

  // Clear queue
  uint32_t nextFree = 0;
  for (uint32_t i = 0; i < state.StateCommands.Count; i++) {
    game_state_command& cmd = state.StateCommands.Commands[i];
    if (!cmd.Processed) {
      state.StateCommands.Commands[nextFree++] = cmd;
    }
  }
  state.StateCommands.Count = nextFree;

  PhysicsSystem::simulate(state.PhysicsSpace, state.SimDelta);
  for (size_t i = 0; i < MapSystem::ENTITY_COUNT; i++) {
    game_entity& entity = state.Map.EntitiesList[i];
    if (entity.Body != nullptr) {
      if (entity.Body->Flags &
          PhysicsSystem::PHYSICS_BODY_FLAG_ENTITY_CONTROLLED) {
        entity.Body->State.Position = entity.Position - entity.BodyOffset;
      } else {
        entity.Position = entity.Body->InterpolatedPosition + entity.BodyOffset;
      }
    }
  }

  AnimationSystem::reset_frame_animation_data(state.Map.EntitiesList,
                                              MapSystem::ENTITY_COUNT);
  AnimationSystem::reset_frame_animation_data(
    state.Map.InstancedEntitiesList, MapSystem::INSTANCED_ENTITY_COUNT);

  state.Animator.DeltaTime = state.SimDelta;
  AnimationSystem::update_animator(&gameMemory, state.Animator);

  for (size_t i = 0; i < MapSystem::ENTITY_COUNT; i++) {
    game_entity& entity = state.Map.EntitiesList[i];
    AnimationSystem::validate_weights(
      entity.ActiveAnimations, entity.ActiveAnimationCount, state.Animator);
    for (uint32_t j = 0; j < entity.ActiveAnimationCount; j++) {
      AnimationSystem::run_id runId = entity.ActiveAnimations[j];
      animation_run* run = AnimationSystem::get_run(runId, state.Animator);
      if (!AnimationSystem::run_active(run) || run->Weight == 0.0f) {
        continue;
      }

      for (uint32_t c = 0; c < run->ChannelCount; c++) {
        AnimationSystem::animation_update_transform* transform =
          run->ChannelTransforms + c;
        Asset::model_bone* targetBone = entity.Bones + transform->BoneIndex;
        switch (transform->PathType) {
          case Asset::TRANSFORMATION:
            targetBone->Translation += transform->NewTranslation * run->Weight;
            targetBone->Rotation += transform->NewRotation * run->Weight;
            targetBone->Scale += transform->NewScale * run->Weight;
            break;
          case Asset::TRANSLATION:
            targetBone->Translation += transform->NewTranslation * run->Weight;
            break;
          case Asset::ROTATION:
            targetBone->Rotation =
              slerp(targetBone->Rotation, transform->NewRotation, run->Weight);
            break;
          case Asset::SCALE:
            targetBone->Scale += transform->NewScale * run->Weight;
            break;
        }
      }
    }
    AnimationSystem::update_bone_transforms(entity);
  }
  for (size_t i = 0; i < MapSystem::INSTANCED_ENTITY_COUNT; i++) {
    instanced_entity& entity = state.Map.InstancedEntitiesList[i];
    AnimationSystem::validate_weights(
      entity.ActiveAnimations, entity.ActiveAnimationCount, state.Animator);
    for (uint32_t j = 0; j < entity.ActiveAnimationCount; j++) {
      AnimationSystem::run_id runId = entity.ActiveAnimations[j];
      animation_run* run = AnimationSystem::get_run(runId, state.Animator);
      for (uint32_t c = 0; c < run->ChannelCount; c++) {
        AnimationSystem::animation_update_transform* transform =
          run->ChannelTransforms + c;
        Asset::model_bone* targetBone = entity.Bones + transform->BoneIndex;
        switch (transform->PathType) {
          case Asset::TRANSFORMATION:
            targetBone->Translation += transform->NewTranslation;
            targetBone->Rotation += transform->NewRotation;
            targetBone->Scale += transform->NewScale;
            break;
          case Asset::TRANSLATION:
            targetBone->Translation += transform->NewTranslation;
            break;
          case Asset::ROTATION:
            targetBone->Rotation += transform->NewRotation;
            break;
          case Asset::SCALE:
            targetBone->Scale += transform->NewScale;
            break;
        }
      }
    }
    AnimationSystem::update_bone_transforms(entity);
  }

  push_render_map(renderContext, &state.Map);
#if 0 // Animation debug
    static game_entity snnnnnnnek = create_entity(&state.Assets->SneikModel);
    static animation_run run =
        AnimationSystem::setup_animation(state.Assets->SneikModel.Animations[0], snnnnnnnek, AnimationSystem::ANIM_LOOP_STYLE_WRAP);
    static bool kek = false; if (!kek) { kek = true; set_animation(run, state.Animator); }
    add_rendercommand(renderContext, snnnnnnnek);
add_debug_rendercommand(renderContext, snnnnnnnek);
#endif

// Debug rendering
// add_debug_rendercommand(renderContext, state.Map.Entities.Offense);
#if 0
  add_debug_rendercommand(renderContext, &state.PhysicsSpace);
#endif

#if 1
  if (state.Phase != game_phase::REPLAYING &&
      state.Phase != game_phase::SETUP && state.Phase != game_phase::RESULT) {
    UISystem::do_game_ui(state, &state.UIContext);
  }
#endif
#if HOKI_DEV && 1
  if (state.ShowDebugUI) {
    UISystem::do_debug_ui(state, &state.UIContext);
  }
#endif
  // finish UI
  UISystem::reset_context(&state.UIContext, renderContext);

#if HOKI_DEV
  if (state.DebugCameraActive) {
    push_render_update_camera(renderContext, &state.DebugCamera);
  } else {
    if (state.Phase == game_phase::REPLAYING) {
      if (state.GoalTime < state.SimTime - state.ReadyTime) {
        camera_update_entity(state.Map.ReplayCamera);
      }
      push_render_update_camera(renderContext, &state.Map.ReplayCamera);
    } else {
      push_render_update_camera(renderContext, &state.Map.GameCamera);
    }
  }
#else
  if (state.Phase == game_phase::REPLAYING) {
    camera_update_entity(state.Map.ReplayCamera);
    push_render_update_camera(renderContext, &state.Map.ReplayCamera);
  } else {
    push_render_update_camera(renderContext, &state.Map.GameCamera);
  }
#endif
  push_setup_ui_context(renderContext, &state.UIContext);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
  game_state* state = (game_state*)gameMemory->PermanentStorage;
#if HOKI_SOUND
  SoundSystem::FillSoundBuffer(state, soundBuffer, gameMemory);
#endif
}
