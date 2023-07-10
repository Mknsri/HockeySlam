#ifndef ASSET_MODEL_H
#define ASSET_MODEL_H

#include "asset_texture.h"
#include "asset_material.h"

namespace Asset {
static const uint32_t MAX_BONES_PER_VERTEX = 4;
static const uint32_t MAX_INTERPOLATED_FRAMES = 4;

struct animation;

struct primitive_data
{
  v3 Position;
  v2 TextureCoord;
  v3 Normal;
  v3 Tangent;
  uint16_t BoneIds[MAX_BONES_PER_VERTEX];
  float BoneWeights[MAX_BONES_PER_VERTEX];
};

struct model_primitive
{
  texture* Texture;
  material* Material;

  size_t IndexCount;
  size_t IndexOffsetBytes;
};

struct model_bone
{
  string Name;

  v3 Translation;
  quat Rotation;
  v3 Scale;

  mat4x4 InverseBind;
  mat4x4 Transform;
  uint32_t NodeId;
};

struct model_mesh
{
  const char* Name;

  model_primitive* Primitives;
  size_t PrimitiveCount;
};

struct model_node
{
  string Name;
  model_mesh* Mesh;
  model_node* Parent;
  int32_t BoneId;
  bool Skinned;

  mat4x4 Transform;
  v3 Translation;
  quat Rotation;
  v3 Scale;
};

struct model
{
  const char* Name;

  primitive_data* Vertices;
  size_t VertexCount;

  uint16_t* Indices;
  size_t IndexCount;

  model_mesh* Meshes;
  size_t MeshCount;

  model_node* Nodes;
  size_t NodeCount;

  texture* Textures;
  size_t TextureCount;

  material* Materials;
  size_t MaterialCount;

  animation* Animations;
  size_t AnimationCount;

  model_bone* Bones;
  size_t BoneCount;

  mat4x4 GlobalInverseTransform;
};

}

#endif // ASSET_MODEL_H