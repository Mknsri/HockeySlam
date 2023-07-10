#ifndef OGL_RENDER_H
#define OGL_RENDER_H

#include <stdint.h>
#include <cmath>

#include "../game/game_render.h"
#include "../game/game_math.h"

static const int32_t HOKI_OGL_EXTENSIONS_OK = 0;
static const int32_t HOKI_OGL_EXTENSIONS_FAILED = 1;
static const int32_t HOKI_OGL_NO_ID = 0;
static const int32_t HOKI_OGL_INVALID_ID = -1;

const size_t MAX_LIGHTS = 4;

enum shader_uniform_type
{
  SHADER_UNIFORM_UNSET,
  SHADER_UNIFORM_INT,
  SHADER_UNIFORM_VEC2,
  SHADER_UNIFORM_VEC3,
  SHADER_UNIFORM_VEC4,
  SHADER_UNIFORM_MAT4,
  SHADER_UNIFORM_BOOL,
  SHADER_UNIFORM_MAT4_ARRAY,
  SHADER_UNIFORM_STRUCT_MATERIAL,
  SHADER_UNIFORM_SAMPLER2D
};

struct shader_uniform
{
  int32_t Id;
  shader_uniform_type Type;
#if HOKI_DEV
  char* Name;
#endif
};

struct shader_uniform_light_base
{
  int32_t ColorId;
  int32_t AmbientId;
  int32_t DiffuseId;
  int32_t SpecularId;
};

struct shader_uniform_light_dir_light : shader_uniform_light_base
{
  int32_t DirectionId;
};

struct shader_uniform_light_point_light : shader_uniform_light_base
{
  int32_t PositionId;
  int32_t ConstantId;
  int32_t LinearId;
  int32_t QuadraticId;
};

struct shader_uniform_lights
{
  shader_uniform_light_dir_light DirLight;
  shader_uniform_light_point_light PointLights[MAX_LIGHTS];
  size_t NextAvailablePointLight; // Tracks which light data to insert next
};

struct shader_uniform_material
{
  int32_t ShineId;

  int32_t AlbedoId;
  int32_t AlbedoMapId;
  int32_t UseAlbedoMapId;

  int32_t MetallicId;
  int32_t MetallicMapId;
  int32_t UseMetallicMapId;

  int32_t RoughnessId;
  int32_t RoughnessMapId;
  int32_t UseRoughnessMapId;
};

struct ogl_shader_base
{
  int32_t Id;
  shader_uniform ModelMatrix;
  shader_uniform ViewProjection;
};

struct ogl_skinned_shader : ogl_shader_base
{
  shader_uniform HasBones;
  shader_uniform Bones;
  shader_uniform LightSpaceMatrix;
  shader_uniform CameraPos;
  shader_uniform_lights Lights;
  shader_uniform_material Material;
  shader_uniform ShadowMap;
};

struct ogl_shader_animated_mesh : ogl_skinned_shader
{
  shader_uniform TextureDiffuse1;
  shader_uniform TextureNormal1;
};

struct ogl_shader_pbr : ogl_skinned_shader
{
  shader_uniform TextureDiffuse1;
};

struct ogl_shader_pbr_instanced : ogl_shader_pbr
{
  shader_uniform InstanceCount;
  shader_uniform InstanceSpacing;
};

struct ogl_shader_simple : ogl_shader_base
{
  shader_uniform ObjectColor;
};

struct ogl_shader_simple_textured : ogl_shader_base
{
  shader_uniform TextureDiffuse1;
};

struct ogl_shader_ui : ogl_shader_base
{
  shader_uniform TextureDiffuse1;
  shader_uniform TextureCoordOffset;
};

struct ogl_shader_text : ogl_shader_base
{
  shader_uniform TextColor;
  shader_uniform CharacterTexture;
};

struct ogl_shader_fill_reveal : ogl_shader_base
{
  shader_uniform TextColor;
};

struct render_context
{
  bool Initialized;
#if HOKI_DEV
  int32_t WeightedShaderProgram;
#endif
  ogl_shader_simple SimpleShader;
  ogl_shader_simple_textured SimpleTexturedShader;
  ogl_shader_animated_mesh AnimatedMeshShader;
  ogl_shader_text TextShader;
  ogl_shader_animated_mesh ShadowShader;
  ogl_shader_fill_reveal FillRevealShader;
  ogl_shader_ui UIShader;
  ogl_shader_pbr PbrShader;
  ogl_shader_pbr_instanced PbrInstancedShader;

  int32_t PointPrimitive;
  int32_t LinePrimitive;
  int32_t RectPrimitive;
  int32_t UIRectPrimitive;
  int32_t CubePrimitive;
  int32_t SpherePrimitive;
  int32_t TetrahedronPrimitive;
  int32_t PyramidPrimitive;

  mat4x4 ProjectionMatrix;
  mat4x4 ProjectionInverse;
  mat4x4 PerspectiveMatrix;

  mat4x4 ViewMatrix;

  render_command_buffer Commands;
  hash_table* RenderableStore;

  int32_t ShadowMapNearTextureId;
  int32_t ShadowMapNearFbo;
  int32_t ShadowMapFarTextureId;
  int32_t ShadowMapFarFbo;
  mat4x4 LightViewMatrix;
  mat4x4 LightProjectionMatrix;

  mat4x4 DebugProjection;
  bool HighQuality;

  mat4x4 ShadowNearProjection;
  mat4x4 ShadowFarProjection;
  mat4x4 ShadowOrthoProjection;
  float ShadowOrthoRadius;
};

#define RENDERER_MAIN(name)                                                    \
  void name(game_window_info& windowInfo, render_context& context)
typedef RENDERER_MAIN(renderer_main);
RENDERER_MAIN(RendererMainStub) {}

#define RENDERER_LOAD_EXTENSIONS(name) int name()
typedef RENDERER_LOAD_EXTENSIONS(renderer_load_extensions);
RENDERER_LOAD_EXTENSIONS(RendererLoadExtensionsStub)
{
  return HOKI_OGL_EXTENSIONS_OK;
}

#endif