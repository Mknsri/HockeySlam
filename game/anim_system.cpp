#include "anim_system.h"

namespace AnimationSystem {
static run_id NextRunId;

void reset_frame_animation_data(game_entity* entityPtr, size_t entityCount)
{
  for (size_t i = 0; i < entityCount; i++) {
    game_entity& entity = entityPtr[i];
    for (uint32_t j = 0; j < entity.BoneCount; j++) {
      entity.Bones[j].Scale = V3_ZERO;
      entity.Bones[j].Translation = V3_ZERO;
      entity.Bones[j].Rotation = QUAT_ZERO;
#if 0
      // TODO: Check if these need to be reset
      entity.Bones[j].Transform = MAT4_ZERO;
      entity.BoneTransforms[j] = MAT4_ZERO;
#endif
    }
  }
}

void reset_frame_animation_data(instanced_entity* entityPtr, size_t entityCount)
{
  for (size_t i = 0; i < entityCount; i++) {
    instanced_entity& entity = entityPtr[i];
    for (uint32_t j = 0; j < entity.BoneCount; j++) {
      entity.Bones[j].Scale = V3_ZERO;
      entity.Bones[j].Translation = V3_ZERO;
      entity.Bones[j].Rotation = QUAT_ZERO;
#if 0
      // TODO: Check if these need to be reset
      entity.Bones[j].Transform = MAT4_ZERO;
      entity.BoneTransforms[j] = MAT4_ZERO;
#endif
    }
  }
}

void reset_animator(animator& animator)
{
  for (size_t i = 0; i < ANIMATOR_MAX_ANIMATIONS; i++) {
    animator.RunningAnimations[i].Loops = 0;
    animator.RunningAnimations[i].EntityCount = 0;
    animator.RunningAnimations[i].Animation = nullptr;
  }
  animation_run* emptyRun = animator.RunningAnimations;
  emptyRun->Id = ANIMATION_NO_ID;
  emptyRun->Animation = &EMPTY_ANIMATION;
  emptyRun->StartDelay = 0.0f;
  emptyRun->CurrentTime = 0.0f;
  emptyRun->ChannelKeyframes = nullptr;
  emptyRun->ChannelTransforms = nullptr;
  emptyRun->ChannelCount = 0;
  emptyRun->Speed = 1.0f;
  emptyRun->Loops = ANIMATION_LOOPS_INFINITE;
  emptyRun->LoopStyle = animation_loop_style::WRAP;
  emptyRun->Weight = 0;
  emptyRun->EntityCount = 999;
}

animation_run* get_run(const run_id runId, const animator& animator)
{
  animation_run* run = (animation_run*)animator.RunningAnimations +
                       (runId % ANIMATOR_MAX_ANIMATIONS);
  if (run->Id == runId) {
    return run;
  }

  return (animation_run*)&animator.RunningAnimations[0];
}

bool run_slot_free(const animation_run* run)
{
  return run == nullptr || run->Loops <= 0 || run->EntityCount <= 0;
}

bool run_active(const animation_run* run)
{
  return !run_slot_free(run) && run->StartDelay <= 0.0f;
}

const animation& find_animation(const char* name,
                                const animation* animations,
                                const size_t animationCount)
{
  const animation* animation = nullptr;
  for (size_t i = 0; i < animationCount; i++) {
    if (strcmp(animations[i].Name.Value, name) == 0) {
      animation = animations + i;
      break;
    }
  }

  HOKI_WARN_MESSAGE(animation != nullptr, "Cant find: %s", name);
  if (animation == nullptr) {
    return EMPTY_ANIMATION;
  }

  return *animation;
}

run_id setup_animation(const animation& animation, animator& animator)
{
  animation_run newRun = {};
  newRun.Id = ANIMATION_NO_ID;
  newRun.Animation = &animation;
  newRun.Speed = ANIMATION_DEFAULT_SPEED;
  newRun.Loops = ANIMATION_LOOPS_INFINITE;
  newRun.LoopStyle = animation_loop_style::WRAP;
  newRun.Weight = ANIMATION_FULL_WEIGHT;
  newRun.ChannelCount = animation.ChannelCount;

  HOKI_ASSERT(newRun.StartDelay >= 0.0f);
  animation_run* handle = nullptr;

  uint32_t index = 0;
  for (size_t i = 0; i < ANIMATOR_MAX_ANIMATIONS; i++) {
    animation_run& runSlot = animator.RunningAnimations[i];
    if (run_slot_free(&runSlot)) {
      newRun.ChannelKeyframes = (uint32_t*)reallocate_t(
        runSlot.ChannelKeyframes,
        sizeof(uint32_t) * newRun.Animation->ChannelCount);
      newRun.ChannelTransforms = (animation_update_transform*)reallocate_t(
        runSlot.ChannelTransforms,
        sizeof(animation_update_transform) * newRun.Animation->ChannelCount);
      for (size_t c = 0; c < newRun.Animation->ChannelCount; c++) {
        newRun.ChannelKeyframes[c] = 0;
        newRun.ChannelTransforms[c] = {};
      }
      runSlot = newRun;
      handle = &runSlot;
      index = (uint32_t)i;
      break;
    }
  }
  HOKI_ASSERT_MESSAGE(handle != nullptr, "Animator capacity exceeded", 0);

  handle->Id = index + (++NextRunId * ANIMATOR_MAX_ANIMATIONS);

  return handle->Id;
}

run_id setup_animation(const char* name,
                       const animation* animations,
                       const size_t animationCount,
                       animator& animator)
{
  const animation& animation = find_animation(name, animations, animationCount);
  return setup_animation(animation, animator);
}

void push_animation_to_group(animation_group& group,
                             const char* const name,
                             const animation* animations,
                             const size_t animationCount,
                             const float weight)
{
  const animation& anim = find_animation(name, animations, animationCount);

  group.Animations[group.Size] = &anim;
  group.Weights[group.Size] = weight;
  group.Size++;
}

void validate_weights(run_id* animationIds,
                      const size_t animationCount,
                      const animator& animator)
{
#if 1
  if (animationCount < 1) {
    return;
  }

  bool anyActive = false;
  float weight = 0.0f;
  for (size_t i = 0; i < animationCount; i++) {
    animation_run* run = get_run(animationIds[i], animator);
    if (run_active(run)) {
      anyActive = true;
      weight += run->Weight;
    }
  }
  if (!anyActive) {
    return;
  }

  HOKI_ASSERT_MESSAGE(equal_f(weight, 1.0f), "Weight: %f", weight);
#endif
}

void add_animation_for_entity(game_entity& entity,
                              const run_id runId,
                              animator& animator)
{
  animation_run* addedRun = get_run(runId, animator);
  addedRun->EntityCount++;
  entity.ActiveAnimations[entity.ActiveAnimationCount++] = runId;
}

void set_animation_for_entity(game_entity& entity,
                              const run_id runId,
                              animator& animator)
{
  for (size_t i = 0; i < entity.ActiveAnimationCount; i++) {
    animation_run* existingRun = get_run(entity.ActiveAnimations[i], animator);
    if (existingRun->Id == runId) {
      existingRun->EntityCount--;
    }
  }
  entity.ActiveAnimationCount = 0;
  add_animation_for_entity(entity, runId, animator);
}

void set_animation_group_for_entity(game_entity& entity,
                                    animation_group& group,
                                    animator& animator)
{
  if (group.Size == 0) {
    return;
  }

  if (!group.UpdateWeightsOnly) {
    for (size_t i = 0; i < entity.ActiveAnimationCount; i++) {
      animation_run* run = get_run(entity.ActiveAnimations[i], animator);
      run->EntityCount--;
    }
    entity.ActiveAnimationCount = 0;
  }

  for (size_t i = 0; i < group.Size; i++) {
    const animation* animation = group.Animations[i];
    const float animationWeight = group.Weights[i];
    if (group.UpdateWeightsOnly) {
      for (uint32_t j = 0; j < entity.ActiveAnimationCount; j++) {
        animation_run* run = get_run(entity.ActiveAnimations[j], animator);
        if (run_active(run) && run->Animation == animation) {
          run->Weight = animationWeight;
          break;
        }
      }
    } else {
      run_id runId = setup_animation(*animation, animator);
      animation_run* run = get_run(runId, animator);
      run->LoopStyle = group.LoopStyle;
      run->Weight = animationWeight;
      add_animation_for_entity(entity, runId, animator);
    }
  }

  entity.ActiveAnimationCount = group.Size;
  validate_weights(
    entity.ActiveAnimations, entity.ActiveAnimationCount, animator);
}

void update_bone_transforms(const game_entity& entity)
{
  for (uint32_t b = 0; b < entity.BoneCount; b++) {
    model_bone& bone = entity.Bones[b];

    mat4x4 translation = mat4x4_translate(bone.Translation);
    mat4x4 rotation = quat_to_mat4x4(bone.Rotation);
    mat4x4 scale = mat4x4_scale(bone.Scale);
    for (int i = 0; i < 3; i++) {
      HOKI_ASSERT(bone.Scale.E[i] <= 2.0f);
    }

    bone.Transform = translation * rotation * scale;
  }

  for (uint32_t b = 0; b < entity.BoneCount; b++) {
    model_bone& bone = entity.Bones[b];

    mat4x4 globalTransform = IDENTITY_MATRIX;
    model_node* node = &entity.Model->Nodes[bone.NodeId];
    while (node) {
      if (node->BoneId > -1) {
        globalTransform =
          entity.Bones[node->BoneId].Transform * globalTransform;
      } else {
        globalTransform = node->Transform * globalTransform;
      }
      node = node->Parent;
    }

    mat4x4 finalTransform =
      entity.Model->GlobalInverseTransform * globalTransform * bone.InverseBind;
    entity.BoneTransforms[b] = finalTransform;

    // Bone world positions for physics/debug systems
    entity.BoneWorldPositions[b] =
      mat4x4_translate(entity.Position) * quat_to_mat4x4(entity.Rotation) *
      finalTransform * mat4x4_invert(bone.InverseBind);
  }
}

void update_animation_run(animation_run& animationRun, const float deltaTime)
{
  const animation& animation = *animationRun.Animation;

  if (animation.Duration == 0.0f) {
    return;
  }
  if (animationRun.Weight == 0.0f) {
    return;
  }

  const int dir = (int)sign(deltaTime * animationRun.Speed);
  animationRun.CurrentTime += deltaTime * animationRun.Speed;

  if (animationRun.LoopStyle == animation_loop_style::PING_PONG) {
    // TODO
    while (animationRun.CurrentTime < 0.0f ||
           animationRun.CurrentTime > animation.Duration) {
      animationRun.Loops--;
      animationRun.CurrentTime -= (animation.Duration * dir);
    }
  }
  if (animationRun.LoopStyle == animation_loop_style::WRAP) {
    while (animationRun.CurrentTime < 0.0f ||
           animationRun.CurrentTime > animation.Duration) {
      animationRun.Loops--;
      animationRun.CurrentTime -= (animation.Duration * dir);
    }
  }
  if (animationRun.LoopStyle == animation_loop_style::NO_LOOP) {
    bool needsClamping = animationRun.CurrentTime > animation.Duration ||
                         animationRun.CurrentTime < 0.0f;
    if (needsClamping) {
      animationRun.Loops--;
      animationRun.CurrentTime =
        clamp(animationRun.CurrentTime, 0, animation.Duration);
    }
  }

  if (animationRun.Loops <= 0) {
    return;
  }

  for (uint32_t c = 0; c < animation.ChannelCount; c++) {
    animation_channel& channel = animation.Channels[c];

    if (channel.KeyframeCount < 2) {
      continue;
    }

    uint32_t& currentKeyframeIndex =
      (uint32_t&)animationRun.ChannelKeyframes[c];
    int32_t nextKeyframeIndex =
      (currentKeyframeIndex + dir + (int32_t)channel.KeyframeCount) %
      (int32_t)channel.KeyframeCount;

    animation_keyframe keyframe = channel.Keyframes[currentKeyframeIndex];
    animation_keyframe nextKeyframe = channel.Keyframes[nextKeyframeIndex];

    float frameDelta = 0.0f;
    float channelTime = clamp(animationRun.CurrentTime,
                              channel.Keyframes[0].StartTime,
                              channel.Duration);
    while (true) {
      if (dir < 0 && currentKeyframeIndex == 0) {
        keyframe.StartTime += channel.Duration;
      }
      if (dir > 0 && nextKeyframeIndex == 0) {
        nextKeyframe.StartTime += channel.Duration;
      }
      frameDelta = (channelTime - keyframe.StartTime) /
                   (nextKeyframe.StartTime - keyframe.StartTime);

      if (frameDelta >= 0.0 && frameDelta <= 1.0f) {
        break;
      }

      currentKeyframeIndex =
        (currentKeyframeIndex + dir + (int32_t)channel.KeyframeCount) %
        (int32_t)channel.KeyframeCount;
      nextKeyframeIndex =
        (currentKeyframeIndex + dir + (int32_t)channel.KeyframeCount) %
        (int32_t)channel.KeyframeCount;
      keyframe = channel.Keyframes[currentKeyframeIndex];
      nextKeyframe = channel.Keyframes[nextKeyframeIndex];
    }

    v3 interpolatedTranslation =
      lerp(keyframe.Translation, nextKeyframe.Translation, frameDelta);
    quat interpolatedRotation =
      slerp(keyframe.Rotation, nextKeyframe.Rotation, frameDelta);
    v3 interpolatedScale = lerp(keyframe.Scale, nextKeyframe.Scale, frameDelta);

    animation_update_transform* result = animationRun.ChannelTransforms + c;
    result->BoneIndex = channel.BoneIndex;
    result->PathType = channel.PathType;
    switch (channel.PathType) {
      case Asset::TRANSFORMATION:
        result->NewTranslation = interpolatedTranslation;
        result->NewRotation = interpolatedRotation;
        result->NewScale = interpolatedScale;
        break;
      case Asset::TRANSLATION:
        result->NewTranslation = interpolatedTranslation;
        break;
      case Asset::ROTATION:
#if 0
        HOKI_ASSERT(equal_f(length(result->NewRotation), 1.0f));
        HOKI_ASSERT(equal_f(length(interpolatedRotation), 1.0f));
#endif
        result->NewRotation = interpolatedRotation;
        break;
      case Asset::SCALE:
        result->NewScale = interpolatedScale;
        break;
    }
  }

  return;
}

PLATFORM_WORK_QUEUE_CALLBACK(update_animation_work)
{
  animator_update* updateData = (animator_update*)data;
  animation_run* run = updateData->Run;
  float deltaTime = updateData->DeltaTime;

  if (run->StartDelay > 0.0f) {
    run->StartDelay -= deltaTime;
  }
  if (run_active(run)) {
    update_animation_run(*run, deltaTime);
  }
}

void update_animator(game_memory* gameMemory, animator& animator)
{
  for (size_t a = 0; a < ANIMATOR_MAX_ANIMATIONS; a++) {
    animator_update* updateData = animator.FrameData + a;
    updateData->Run = animator.RunningAnimations + a;

    if (updateData->Run->Driver == animation_driver::TIME) {
      updateData->DeltaTime = animator.DeltaTime;
    } else {
      updateData->DeltaTime = animator.DeltaInput;
    }
#if 0
    update_animation_work(updateData);
#else
    gameMemory->AddWorkEntry(
      gameMemory->WorkQueue, update_animation_work, updateData);
#endif
  }

  gameMemory->CompleteAllQueueWork(gameMemory->WorkQueue);

  animator.DeltaTime = 0.0f;
  animator.DeltaInput = 0.0f;
}
}