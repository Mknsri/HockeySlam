#include "game_entity.h"

void create_entity(const char* name,
                   const Asset::model* const model,
                   game_entity& outEntity)
{
#if HOKI_DEV
  outEntity.Name = name;
#endif
  outEntity.Scale = _v3(1.0f);
  outEntity.Rotation = IDENTITY_ROTATION;
  outEntity.Model = model;

  outEntity.BoneCount = model->BoneCount;
  outEntity.Bones = (Asset::model_bone*)reallocate_t(
    outEntity.Bones, sizeof(Asset::model_bone) * outEntity.BoneCount);

  for (size_t i = 0; i < outEntity.BoneCount; i++) {
    outEntity.Bones[i] = model->Bones[i];
  }

  outEntity.BoneTransforms = (mat4x4*)reallocate_t(
    outEntity.BoneTransforms, sizeof(mat4x4) * outEntity.BoneCount);
  outEntity.BoneWorldPositions = (mat4x4*)reallocate_t(
    outEntity.BoneWorldPositions, sizeof(mat4x4) * outEntity.BoneCount);

  for (size_t i = 0; i < outEntity.BoneCount; i++) {
    outEntity.BoneTransforms[i] = IDENTITY_MATRIX;
    outEntity.BoneWorldPositions[i] = IDENTITY_MATRIX;
  }
}
