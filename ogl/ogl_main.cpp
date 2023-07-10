#include "ogl_main.h"

#ifdef _WIN32
#include <Windows.h>

#include <gl/gl.h>
#include <gl/glext.h>
#include <gl/wglext.h>
#include "ogl_extensions.cpp"
#endif

#include <map>

#include "../game/debug/debug_log.h"
#include "../game/debug/debug_assert.h"

#include "../game/game_math.h"
#include "../game/game_entity.h"
#include "../game/game_assets.h"
#include "../game/asset/asset_texture.h"
#include "../game/asset/asset_material.h"
#include "../game/asset/asset_decal.h"
#include "../game/asset/asset_model.h"
#include "../game/asset/asset_gradient.h"
#include "../game/game_render.h"

#if HOKI_DEV
#include "../game/debug/debug_strings.cpp"
#endif

#include "ogl_shader.cpp"
#include "ogl_primitives.cpp"
#include "ogl_imui.cpp"
#include "ogl_text.cpp"
#include "ogl_physics.cpp"

static const float NEAR_PLANE = 0.1f;
static const float FAR_PLANE = 160.0f;
static const float SHADOW_MAP_SPLIT_DEPTH = 15.0f;

const unsigned int SHADOW_MAP_SIZE = 512;
static const float FOV = 90.0f;
static float ASPECT_RATIO = 1.0f;

static uint32_t BindTexture(const Asset::texture& texture)
{
  uint32_t texobj;

  glGenTextures(1, &texobj);
  glBindTexture(GL_TEXTURE_2D, texobj);

  GLint wrapType;
  switch (texture.WrapType) {
    case Asset::texture_wrap_type::TEXTURE_WRAP_CLAMP:
      wrapType = GL_CLAMP_TO_EDGE;
      break;

    case Asset::texture_wrap_type::TEXTURE_WRAP_REPEAT:
      wrapType = GL_REPEAT;
      break;

    default:
      wrapType = GL_REPEAT;
      break;
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapType);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapType);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  HOKI_ASSERT(texture.Type != Asset::TEXTURE_TYPE_INVALID);
  if (texture.Type == Asset::TEXTURE_TYPE_DDS) {
    const Asset::DDS_Image& image = *texture.DDSImage;
    Asset::DDS_Surface surface = (Asset::DDS_Surface)image.Surfaces[0];
    glCompressedTexImage2D(GL_TEXTURE_2D,
                           0,
                           image.Format,
                           surface.Width,
                           surface.Height,
                           0,
                           surface.Size,
                           surface.Pixels);

    for (uint32_t i = 0; i < image.SurfaceCount; i++) {
      Asset::DDS_Surface mipmapSurface = image.Surfaces[i + 1];

      glCompressedTexImage2D(GL_TEXTURE_2D,
                             i + 1,
                             image.Format,
                             mipmapSurface.Width,
                             mipmapSurface.Height,
                             0,
                             mipmapSurface.Size,
                             mipmapSurface.Pixels);
    }
  } else if (texture.Type == Asset::TEXTURE_TYPE_IMAGE) {
    const Asset::image_texture& image = *texture.Image;
#if HOKI_DEV && !GL_ES_VERSION_3_0
    // apply the name, -1 means NULL terminated
    glObjectLabel(GL_TEXTURE, texobj, -1, texture.Name.Value);
#endif
    int nrComponents = image.Components;
    if (image.Memory) {
      GLenum format = GL_RED;
      if (nrComponents == 1) {
        format = GL_RED;
      } else if (nrComponents == 3) {
        format = GL_RGB;
      } else if (nrComponents == 4) {
        format = GL_RGBA;
      }

      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   format,
                   (int)image.PixelSize.X,
                   (int)image.PixelSize.Y,
                   0,
                   format,
                   GL_UNSIGNED_BYTE,
                   image.Memory);
      glGenerateMipmap(GL_TEXTURE_2D);
    }
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  return texobj;
}

static uint32_t BindModel(const Asset::model& model)
{
  uint32_t VAO = 0, VBO, EBO;

  // create buffers/arrays
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

#if HOKI_DEV && !GL_ES_VERSION_3_0
  // apply the name, -1 means NULL terminated
  glObjectLabel(GL_VERTEX_ARRAY, VAO, -1, model.Name);
#endif

  // load data into vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER,
               model.VertexCount * sizeof(model.Vertices[0]),
               model.Vertices,
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               model.IndexCount * sizeof(model.Indices[0]),
               model.Indices,
               GL_STATIC_DRAW);

  // set the vertex attribute pointers
  // vertex Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,
                        sizeof(v3) / sizeof(float),
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(Asset::primitive_data),
                        (void*)0);

  // vertex texture coords
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,
                        sizeof(v2) / sizeof(float),
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(Asset::primitive_data),
                        (void*)offsetof(Asset::primitive_data, TextureCoord));

  // vertex normals
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2,
                        sizeof(v3) / sizeof(float),
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(Asset::primitive_data),
                        (void*)offsetof(Asset::primitive_data, Normal));

  // vertex tangent
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3,
                        sizeof(v3) / sizeof(float),
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(Asset::primitive_data),
                        (void*)offsetof(Asset::primitive_data, Tangent));

  // bone ids
  glEnableVertexAttribArray(4);
  glVertexAttribIPointer(4,
                         Asset::MAX_BONES_PER_VERTEX,
                         GL_UNSIGNED_SHORT,
                         sizeof(Asset::primitive_data),
                         (void*)offsetof(Asset::primitive_data, BoneIds));

  // bone weights
  glEnableVertexAttribArray(5);
  glVertexAttribPointer(5,
                        Asset::MAX_BONES_PER_VERTEX,
                        GL_FLOAT,
                        GL_FALSE,
                        sizeof(Asset::primitive_data),
                        (void*)offsetof(Asset::primitive_data, BoneWeights));

  glBindVertexArray(0);

  return VAO;
}

static render_command* GetNextCommand(render_command_buffer& buffer, size_t i)
{
  if (i >= buffer.Count) {
    return nullptr;
  }
  return buffer.Commands + i;
}

static void RenderEntity(const game_entity& entity,
                         render_context& context,
                         const ogl_skinned_shader& shader,
                         const bool translucentPass)
{

  renderable* model = (renderable*)get(*context.RenderableStore, entity.Model);
  if (model->RenderId == HOKI_OGL_NO_ID) {
    model->RenderId = BindModel(*entity.Model);
  }

  if (entity.BoneCount > 0) {
    SetUniform(shader.Bones, entity.BoneTransforms, entity.BoneCount);
  }

  glBindVertexArray(model->RenderId);
  mat4x4 viewProjection = context.ProjectionMatrix * context.ViewMatrix;
  SetUniform(shader.ViewProjection, &viewProjection, 1);
  SetUniform(shader.LightSpaceMatrix, &context.LightProjectionMatrix, 1);

  int32_t textureIndex = 0;
  if (context.ShadowMapNearTextureId != HOKI_OGL_NO_ID) {
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    SetSamplerUniform(shader.ShadowMap, textureIndex);
    glBindTexture(GL_TEXTURE_2D, context.ShadowMapNearTextureId);
    textureIndex++;
  }
  for (size_t n = 0; n < entity.Model->NodeCount; n++) {
    Asset::model_node* nodeInfo = entity.Model->Nodes + n;
    if (nodeInfo->Mesh == nullptr) {
      continue;
    }
    Asset::model_mesh& meshInfo = *nodeInfo->Mesh;

    mat4x4 finalTransform = nodeInfo->Transform;
    Asset::model_node* parentNode = nodeInfo->Parent;
    while (!nodeInfo->Skinned && parentNode) {
      if (parentNode->BoneId > -1) {
        finalTransform =
          entity.Bones[parentNode->BoneId].Transform * finalTransform;
      } else {
        finalTransform = parentNode->Transform * finalTransform;
      }

      parentNode = parentNode->Parent;
    }

    mat4x4 modelMatrix = IDENTITY_MATRIX * mat4x4_translate(entity.Position) *
                         quat_to_mat4x4(entity.Rotation) *
                         mat4x4_scale(entity.Scale) * finalTransform;
    SetUniform(shader.ModelMatrix, &modelMatrix, 1);
    SetUniform(shader.HasBones, nodeInfo->Skinned);

    for (size_t p = 0; p < meshInfo.PrimitiveCount; p++) {
      Asset::model_primitive& primitive = meshInfo.Primitives[p];

      // Exit early if translucentPass does not match material translucency
      if (!translucentPass && primitive.Material != nullptr &&
          primitive.Material->Translucent) {
        continue;
      } else if (translucentPass && primitive.Material != nullptr &&
                 !primitive.Material->Translucent) {
        continue;
      }

      // Material texture is prioritized
      const texture* albedoTexture = nullptr;
      const texture* metallicTexture = nullptr;
      const texture* roughnessTexture = nullptr;
      const texture* normalTexture = nullptr;
      if (primitive.Material != nullptr) {
        albedoTexture = primitive.Material->AlbedoMap;
        metallicTexture = primitive.Material->MetallicMap;
        roughnessTexture = primitive.Material->RoughnessMap;
        normalTexture = primitive.Material->NormalMap;
      } else if (entity.Texture != nullptr) {
        albedoTexture = entity.Texture;
      } else if (primitive.Texture != nullptr) {
        albedoTexture = primitive.Texture;
      }

      GLint albedoMapId = HOKI_OGL_INVALID_ID;
      if (albedoTexture != nullptr) {
        renderable* albedoTextureRenderable =
          (renderable*)get(*context.RenderableStore, albedoTexture);
        if (albedoTextureRenderable->RenderId == HOKI_OGL_NO_ID) {
          albedoTextureRenderable->RenderId = BindTexture(*albedoTexture);
        }
        albedoMapId = albedoTextureRenderable->RenderId;
      }
      GLint metallicMapId = HOKI_OGL_INVALID_ID;
      if (metallicTexture != nullptr) {
        renderable* metallicTextureRenderable =
          (renderable*)get(*context.RenderableStore, metallicTexture);
        if (metallicTextureRenderable->RenderId == HOKI_OGL_NO_ID) {
          metallicTextureRenderable->RenderId = BindTexture(*metallicTexture);
        }
        metallicMapId = metallicTextureRenderable->RenderId;
      }
      GLint roughnessMapId = HOKI_OGL_INVALID_ID;
      if (roughnessTexture != nullptr) {
        renderable* roughnessTextureRenderable =
          (renderable*)get(*context.RenderableStore, roughnessTexture);
        if (roughnessTextureRenderable->RenderId == HOKI_OGL_NO_ID) {
          roughnessTextureRenderable->RenderId = BindTexture(*roughnessTexture);
        }
        roughnessMapId = roughnessTextureRenderable->RenderId;
      }

      SetUniform(shader.Material,
                 primitive.Material,
                 albedoMapId,
                 roughnessMapId,
                 metallicMapId,
                 textureIndex,
                 1);

      glDrawElements(GL_TRIANGLES,
                     (GLsizei)primitive.IndexCount,
                     GL_UNSIGNED_SHORT,
                     (GLvoid*)primitive.IndexOffsetBytes);
    }
  }

  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

static void RenderPbrEntityInstanced(const instanced_entity& entity,
                                     render_context& context,
                                     const bool translucentPass)
{

  renderable* model = (renderable*)get(*context.RenderableStore, entity.Model);
  if (model->RenderId == HOKI_OGL_NO_ID) {
    model->RenderId = BindModel(*entity.Model);
  }

  if (entity.BoneCount > 0) {
    SetUniform(context.PbrInstancedShader.Bones,
               entity.BoneTransforms,
               entity.BoneCount);
  }
  SetUniform(context.PbrInstancedShader.InstanceCount, entity.InstanceCount);
  SetUniform(
    context.PbrInstancedShader.InstanceSpacing, &entity.InstanceSpacing, 1);

  glBindVertexArray(model->RenderId);

  mat4x4 viewProjection = context.ProjectionMatrix * context.ViewMatrix;

  SetUniform(context.PbrInstancedShader.ViewProjection, &viewProjection, 1);
  SetUniform(context.PbrInstancedShader.LightSpaceMatrix,
             &context.LightProjectionMatrix,
             1);
  int32_t textureIndex = 0;
  if (context.ShadowMapNearTextureId != HOKI_OGL_NO_ID) {
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    SetSamplerUniform(context.PbrInstancedShader.ShadowMap, textureIndex);
    glBindTexture(GL_TEXTURE_2D, context.ShadowMapNearTextureId);
    textureIndex++;
  }
  for (size_t n = 0; n < entity.Model->NodeCount; n++) {
    Asset::model_node* nodeInfo = entity.Model->Nodes + n;
    if (nodeInfo->Mesh == nullptr) {
      continue;
    }

    Asset::model_mesh& meshInfo = *nodeInfo->Mesh;

    mat4x4 finalTransform = nodeInfo->Transform;
    Asset::model_node* parentNode = nodeInfo->Parent;
    while (!nodeInfo->Skinned && parentNode) {
      if (parentNode->BoneId > -1) {
        finalTransform =
          entity.Bones[parentNode->BoneId].Transform * finalTransform;
      } else {
        finalTransform = parentNode->Transform * finalTransform;
      }

      parentNode = parentNode->Parent;
    }

    mat4x4 modelMatrix = IDENTITY_MATRIX * mat4x4_translate(entity.Position) *
                         quat_to_mat4x4(entity.Rotation) *
                         mat4x4_scale(entity.Scale) * finalTransform;

    SetUniform(context.PbrInstancedShader.ModelMatrix, &modelMatrix, 1);
#if 0
    SetUniform(context.PbrInstancedShader.HasBones, false);
#else
    SetUniform(context.PbrInstancedShader.HasBones, nodeInfo->Skinned);
#endif

    for (size_t p = 0; p < meshInfo.PrimitiveCount; p++) {
      Asset::model_primitive& primitive = meshInfo.Primitives[p];

      // Exit early if translucentPass does not match material translucency
      if (!translucentPass && primitive.Material != nullptr &&
          primitive.Material->Translucent) {
        continue;
      } else if (translucentPass && primitive.Material != nullptr &&
                 !primitive.Material->Translucent) {
        continue;
      }

      // Material texture is prioritized
      const texture* albedoTexture = nullptr;
      const texture* metallicTexture = nullptr;
      const texture* roughnessTexture = nullptr;
      const texture* normalTexture = nullptr;
      if (primitive.Material != nullptr) {
        albedoTexture = primitive.Material->AlbedoMap;
        metallicTexture = primitive.Material->MetallicMap;
        roughnessTexture = primitive.Material->RoughnessMap;
        normalTexture = primitive.Material->NormalMap;
      } else if (entity.Texture != nullptr) {
        albedoTexture = entity.Texture;
      } else if (primitive.Texture != nullptr) {
        albedoTexture = primitive.Texture;
      }

      GLint albedoMapId = HOKI_OGL_INVALID_ID;
      if (albedoTexture != nullptr) {
        renderable* albedoTextureRenderable =
          (renderable*)get(*context.RenderableStore, albedoTexture);
        if (albedoTextureRenderable->RenderId == HOKI_OGL_NO_ID) {
          albedoTextureRenderable->RenderId = BindTexture(*albedoTexture);
        }
        albedoMapId = albedoTextureRenderable->RenderId;
      }
      GLint metallicMapId = HOKI_OGL_INVALID_ID;
      if (metallicTexture != nullptr) {
        renderable* metallicTextureRenderable =
          (renderable*)get(*context.RenderableStore, metallicTexture);
        if (metallicTextureRenderable->RenderId == HOKI_OGL_NO_ID) {
          metallicTextureRenderable->RenderId = BindTexture(*metallicTexture);
        }
        metallicMapId = metallicTextureRenderable->RenderId;
      }
      GLint roughnessMapId = HOKI_OGL_INVALID_ID;
      if (roughnessTexture != nullptr) {
        renderable* roughnessTextureRenderable =
          (renderable*)get(*context.RenderableStore, roughnessTexture);
        if (roughnessTextureRenderable->RenderId == HOKI_OGL_NO_ID) {
          roughnessTextureRenderable->RenderId = BindTexture(*roughnessTexture);
        }
        roughnessMapId = roughnessTextureRenderable->RenderId;
      }

      SetUniform(context.PbrInstancedShader.Material,
                 primitive.Material,
                 albedoMapId,
                 roughnessMapId,
                 metallicMapId,
                 textureIndex,
                 1);

      glDrawElementsInstanced(GL_TRIANGLES,
                              (GLsizei)primitive.IndexCount,
                              GL_UNSIGNED_SHORT,
                              (GLvoid*)primitive.IndexOffsetBytes,
                              entity.InstanceCount);
    }
  }

  HOKI_ASSERT_NO_OPENGL_ERRORS();

  glBindVertexArray(0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

#if HOKI_DEV
static void DEBUG_RenderBones(const game_entity& entity,
                              render_context& context)
{
  const Asset::model& model = *entity.Model;

  glDisable(GL_DEPTH_TEST);

#ifndef GL_ES_VERSION_3_0
  GLint previousPolygonMode;
  glGetIntegerv(GL_POLYGON_MODE, &previousPolygonMode);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif

  for (uint32_t b = 0; b < model.BoneCount; b++) {
    renderable* bone =
      (renderable*)get(*context.RenderableStore, model.Bones + b);
    if (bone->RenderId == HOKI_OGL_NO_ID) {
      bone->RenderId = BindPyramid(context);
    }

    mat4x4 modelMatrix = entity.BoneWorldPositions[b];
    mat4x4 viewProjection = context.ProjectionMatrix * context.ViewMatrix;
    v3 objectColor = _v3(1.0f, 0.5f, 0.25f);

    SetUniform(context.SimpleShader.ModelMatrix, &modelMatrix, 1);
    SetUniform(context.SimpleShader.ViewProjection, &viewProjection, 1);
    SetUniform(context.SimpleShader.ObjectColor, &objectColor, 1);

    // draw mesh
    glBindVertexArray(bone->RenderId);
    glDrawArrays(GL_TRIANGLES, 0, 18);
    glBindVertexArray(0);
  }
#ifndef GL_ES_VERSION_3_0
  glPolygonMode(GL_FRONT_AND_BACK, previousPolygonMode);
#endif
  glEnable(GL_DEPTH_TEST);
}

static void DEBUG_RenderPhysicsBody(const PhysicsSystem::body& body,
                                    render_context& context)
{

  glDisable(GL_DEPTH_TEST);

  switch (body.Type) {
    case PhysicsSystem::PHYSICS_BODY_TYPE_AABB:
      DEBUG_RenderPhysicsAABB(body, context);
      break;

    case PhysicsSystem::PHYSICS_BODY_TYPE_RAY:
      DEBUG_RenderPhysicsRay(body, context);
      break;

    default:
      HOKI_ASSERT(false);
      break;
  }

  glEnable(GL_DEPTH_TEST);
  glBindVertexArray(0);
}

static void SetWireframe()
{
#ifndef GL_ES_VERSION_3_0
  GLint previousPolygonMode;
  glGetIntegerv(GL_POLYGON_MODE, &previousPolygonMode);
  if (previousPolygonMode == GL_FILL) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
#endif
}

static void DEBUG_RenderLight(const game_light& light, render_context& context)
{
  glUseProgram(context.SimpleShader.Id);
  uint32_t lightMeshId = BindPyramid(context);
  mat4x4 viewProjection = context.ProjectionMatrix * context.ViewMatrix;
  SetUniform(context.SimpleShader.ViewProjection, &viewProjection, 1);

  mat4x4 modelMatrix = IDENTITY_MATRIX;

  switch (light.LightType) {
    case LIGHT_TYPE_DIRECTION: {
      v3 rotationFromLight =
        _v3(light.Vector.X + 180.0f, light.Vector.Y, light.Vector.Z);
      modelMatrix = mat4x4_translate(_v3(0.0f, 3.0f, 0.0f)) *
                    mat4x4_rotate(rotationFromLight) * mat4x4_scale(_v3(1.0f));
    } break;

    case LIGHT_TYPE_POINT: {
      modelMatrix = mat4x4_translate(light.Vector) * mat4x4_scale(_v3(1.0f));
    } break;

    default:
      HOKI_ASSERT(false);
  }

  SetUniform(context.SimpleShader.ModelMatrix, &modelMatrix, 1);
  SetUniform(context.SimpleShader.ObjectColor, &light.Color, 1);

  glBindVertexArray(lightMeshId);
  glDrawArrays(GL_TRIANGLES, 0, 36);
}

#endif

static void UpdateCamera(const game_camera& camera, render_context& context)
{
  context.ViewMatrix = look_at(camera.Position, camera.Target);

  if (context.HighQuality) {
    SetUniform(context.PbrShader.CameraPos, &camera.Position, 1);
  } else {
    SetUniform(context.AnimatedMeshShader.CameraPos, &camera.Position, 1);
  }
}

static void RenderLight(shader_uniform_lights* targetLights,
                        const game_light* light,
                        render_context& context)
{
  SetUniform(*targetLights, light);
}

static void SetupShadowmaps(render_context& context)
{
  GLuint depthMapFbo;
  glGenFramebuffers(1, &depthMapFbo);

  GLuint depthMapTextureId;
  glGenTextures(1, &depthMapTextureId);
  glBindTexture(GL_TEXTURE_2D, depthMapTextureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_DEPTH_COMPONENT24,
               SHADOW_MAP_SIZE,
               SHADOW_MAP_SIZE,
               0,
               GL_DEPTH_COMPONENT,
               GL_UNSIGNED_INT,
               nullptr);

  // GL_NEAREST works on Adreno
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

#ifndef GL_ES_VERSION_3_0
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  const v4 borderColor = _v4(1.0f);
  glTexParameterfv(
    GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (GLfloat*)&borderColor.E[0]);
#endif

  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTextureId, 0);

#ifdef _WIN32
  glDrawBuffer(GL_NONE);
#else
  glDrawBuffers(GL_NONE, nullptr);
#endif
  glReadBuffer(GL_NONE);

  GLenum bufferComplete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  HOKI_ASSERT(bufferComplete == GL_FRAMEBUFFER_COMPLETE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  context.ShadowMapNearTextureId = depthMapTextureId;
  context.ShadowMapNearFbo = depthMapFbo;
}

static void RenderShadowmap(const MapSystem::map map,
                            render_context& context,
                            const game_window_info& windowInfo)
{
  if (context.ShadowMapNearFbo == HOKI_OGL_NO_ID) {
    SetupShadowmaps(context);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, context.ShadowMapNearFbo);
  glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
  glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glCullFace(GL_FRONT);

  v3 mid =
    get_midpoint_for_frustum(context.ShadowNearProjection * context.ViewMatrix);
  v3 target = mid + map.Lights[0].Vector;

  context.LightViewMatrix = look_at(mid, target);

  context.LightProjectionMatrix =
    context.ShadowOrthoProjection * context.LightViewMatrix;
#if 1
  // Avoid shadow moving when only the camera moves
  float worldUnitsPerTexel = 2.0f / SHADOW_MAP_SIZE;
  for (int i = 12; i < 15; i++) {
    mat4x4& matrix = context.LightProjectionMatrix;
    matrix.S[i] = quantize_f(matrix.S[12], worldUnitsPerTexel);
  }
#endif

  SetUniform(
    context.ShadowShader.ViewProjection, &context.LightProjectionMatrix, 1);

  for (size_t i = 0; i < MapSystem::ENTITY_COUNT; i++) {
    const game_entity& entity = map.EntitiesList[i];

    if (&entity == &map.Entities.Field) {
      continue;
    }

    if (entity.BoneCount > 0) {
      SetUniform(
        context.ShadowShader.Bones, entity.BoneTransforms, entity.BoneCount);
    }

    renderable* model =
      (renderable*)get(*context.RenderableStore, entity.Model);
    if (model->RenderId == HOKI_OGL_NO_ID) {
      model->RenderId = BindModel(*entity.Model);
    }

    glBindVertexArray(model->RenderId);

    for (size_t n = 0; n < entity.Model->NodeCount; n++) {
      Asset::model_node* nodeInfo = entity.Model->Nodes + n;
      if (nodeInfo->Mesh == nullptr) {
        continue;
      }
      Asset::model_mesh& meshInfo = *nodeInfo->Mesh;

      mat4x4 finalTransform = nodeInfo->Transform;
      Asset::model_node* parentNode = nodeInfo->Parent;
      while (!nodeInfo->Skinned && parentNode) {
        if (parentNode->BoneId > -1) {
          finalTransform =
            entity.Bones[parentNode->BoneId].Transform * finalTransform;
        } else {
          finalTransform = parentNode->Transform * finalTransform;
        }

        parentNode = parentNode->Parent;
      }

      mat4x4 modelMatrix = IDENTITY_MATRIX * mat4x4_translate(entity.Position) *
                           quat_to_mat4x4(entity.Rotation) *
                           mat4x4_scale(entity.Scale) * finalTransform;

      SetUniform(context.ShadowShader.ModelMatrix, &modelMatrix, 1);
      SetUniform(context.ShadowShader.HasBones, nodeInfo->Skinned);

      for (size_t p = 0; p < meshInfo.PrimitiveCount; p++) {
        Asset::model_primitive& primitive = meshInfo.Primitives[p];
        if (primitive.Material != nullptr && primitive.Material->Translucent) {
          continue;
        }

        glDrawElements(GL_TRIANGLES,
                       (GLsizei)primitive.IndexCount,
                       GL_UNSIGNED_SHORT,
                       (GLvoid*)primitive.IndexOffsetBytes);
      }
    }
    glBindVertexArray(0);
  }

  // glCullFace(GL_BACK);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, windowInfo.Width, windowInfo.Height);

#if HOKI_DEV && 0
  // Draw rect with shadow buffer contents
  glUseProgram(context.SimpleTexturedShader.Id);
  uint32_t plane = BindRect(context);
  mat4x4 shadowmapPlaneModelMatrix =
    IDENTITY_MATRIX * mat4x4_translate(_v3(0.0f)) * mat4x4_scale(_v3(0.9f));
  SetUniform(
    context.SimpleTexturedShader.ModelMatrix, &shadowmapPlaneModelMatrix, 1);
  SetUniform(context.SimpleTexturedShader.ViewProjection, &IDENTITY_MATRIX, 1);
  // draw mesh
  glBindVertexArray(plane);
  glBindTexture(GL_TEXTURE_2D, context.ShadowMapNearTextureId);
  glActiveTexture(GL_TEXTURE0);
  glDrawArrays(GL_TRIANGLES, 0, 6);
#endif
}

static void CameraPass(render_context& context)
{
  for (uint32_t i = 0;
       render_command* command = GetNextCommand(context.Commands, i++);) {
    switch (command->Type) {
      case RENDER_COMMAND_TYPE_CAMERA:
        UpdateCamera(*command->Camera, context);
        break;
    }
    HOKI_ASSERT_NO_OPENGL_ERRORS();
  }
}

static void EntityPass(render_context& context, ogl_skinned_shader* shader)
{
  for (uint32_t i = 0;
       render_command* command = GetNextCommand(context.Commands, i++);) {

    switch (command->Type) {
      case RENDER_COMMAND_TYPE_ENTITY:
        RenderEntity(*command->Entity, context, *shader, false);
        break;

      case RENDER_COMMAND_TYPE_LIGHT:
        RenderLight(&shader->Lights, command->Light, context);
        break;
    }
  }
  HOKI_ASSERT_NO_OPENGL_ERRORS();
}

static void Initialize(render_context& context, game_window_info& windowInfo)
{

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  // PNG Transparency
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  ASPECT_RATIO = (float)windowInfo.Width / (float)windowInfo.Height;
  context.PerspectiveMatrix =
    perspective_matrix(FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);
  context.ProjectionMatrix = context.PerspectiveMatrix;

  // Shadow mapping stuff
  context.ShadowNearProjection =
    perspective_matrix(FOV, ASPECT_RATIO, NEAR_PLANE, SHADOW_MAP_SPLIT_DEPTH);
  // context.ShadowFarProjection =
  //  perspective_matrix(FOV, ASPECT_RATIO, SHADOW_MAP_SPLIT_DEPTH,
  //  FAR_PLANE);

  // calculate frustum size
  mat4x4 projInv = mat4x4_invert(context.ShadowNearProjection);
  v4 LBN = projInv * _v4(-1.0f, -1.0f, -1.0f, 1.0f);
  LBN = LBN / LBN.W;
  v4 RTF = projInv * _v4(1.0f, 1.0f, 1.0f, 1.0f);
  RTF = RTF / RTF.W;
  context.ShadowOrthoRadius = std::ceil(length(LBN.XYZ - RTF.XYZ));
  context.ShadowOrthoProjection =
    orthographic_matrix(-context.ShadowOrthoRadius,
                        context.ShadowOrthoRadius,
                        context.ShadowOrthoRadius,
                        -context.ShadowOrthoRadius,
                        -context.ShadowOrthoRadius,
                        context.ShadowOrthoRadius);
}

extern "C" RENDERER_MAIN(RendererMain)
{
  if (windowInfo.Width == 0 || windowInfo.Height == 0) {
    return;
  }

  if (!context.Initialized) {
    return;
  }

  // glEnable(GL_FRAMEBUFFER_SRGB);
#if HOKI_DEV
  glClearColor(0.0f, 0.5f, 1.0f, 1.0f);
#else
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
#endif
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glViewport(0, 0, windowInfo.Width, windowInfo.Height);

#if HOKI_DEV
  // calculate each frame
  Initialize(context, windowInfo);
#endif

  // SETUP PASS
  for (uint32_t i = 0;
       render_command* command = GetNextCommand(context.Commands, i++);) {

    switch (command->Type) {
      case RENDER_COMMAND_TYPE_INITIALIZE:
        Initialize(context, windowInfo);
        break;

      case RENDER_COMMAND_TYPE_CREATE_SHADER:
        CreateShader(*command->ShaderProgram, context);
        break;
    }

    HOKI_ASSERT_NO_OPENGL_ERRORS();
  }
  // CAMERA PASS
  if (context.HighQuality) {
    glUseProgram(context.PbrShader.Id);
    CameraPass(context);
    glUseProgram(context.PbrInstancedShader.Id);
    CameraPass(context);
  } else {
    glUseProgram(context.AnimatedMeshShader.Id);
    CameraPass(context);
  }

  // SHADOW PASS
  glUseProgram(context.ShadowShader.Id);
  for (uint32_t i = 0;
       render_command* command = GetNextCommand(context.Commands, i++);) {
    switch (command->Type) {
      case RENDER_COMMAND_TYPE_SHADOW: {
        RenderShadowmap(*command->Map, context, windowInfo);
      } break;
    }

    HOKI_ASSERT_NO_OPENGL_ERRORS();
  }

  // INSTANCED PASS
  if (context.HighQuality) {
    glUseProgram(context.PbrInstancedShader.Id);
    for (uint32_t i = 0;
         render_command* command = GetNextCommand(context.Commands, i++);) {

      switch (command->Type) {
        case RENDER_COMMAND_TYPE_INSTANCED:
          RenderPbrEntityInstanced(*command->InstancedEntity, context, false);
          break;

        case RENDER_COMMAND_TYPE_LIGHT:
          RenderLight(
            &context.PbrInstancedShader.Lights, command->Light, context);
          break;

        default:
          continue;
      }

      HOKI_ASSERT_NO_OPENGL_ERRORS();
    }
  }

  // ENTITY PASS
  ogl_skinned_shader entityShader;
  if (context.HighQuality) {
    entityShader = context.PbrShader;
  } else {
    entityShader = context.AnimatedMeshShader;
  }
  glUseProgram(entityShader.Id);
  EntityPass(context, &entityShader);

  // TRANSLUCENT PASS
  glEnable(GL_BLEND);
  for (uint32_t i = 0;
       render_command* command = GetNextCommand(context.Commands, i++);) {
    switch (command->Type) {
      case RENDER_COMMAND_TYPE_TRANSLUCENT:
        RenderEntity(*command->Entity, context, entityShader, true);
        break;
    }

    HOKI_ASSERT_NO_OPENGL_ERRORS();
  }

  // UI PASS
  glUseProgram(context.UIShader.Id);
  glDisable(GL_DEPTH_TEST);
  for (uint32_t i = 0;
       render_command* command = GetNextCommand(context.Commands, i++);) {
    switch (command->Type) {
      case RENDER_COMMAND_TYPE_UI_BUTTON:
        RenderButton(*command->Button, context);
        break;
      case RENDER_COMMAND_TYPE_UI_ICON:
        RenderIcon(*command->Icon, context);
        break;
      case RENDER_COMMAND_TYPE_UI_CONTEXT:
        SetupUIContext(*command->UIContext, windowInfo, context);
        break;
      case RENDER_COMMAND_TYPE_UI_TOGGLE:
        RenderToggle(*command->Toggle, context);
        break;
      case RENDER_COMMAND_TYPE_UI_JOYSTICK:
        RenderJoystick(*command->Joystick, context);
        break;
      case RENDER_COMMAND_TYPE_UI_SLIDER:
        RenderSlider(*command->Slider, context);
        break;
    }
  }
  glEnable(GL_DEPTH_TEST);

  // TEXT PASS
  glUseProgram(context.TextShader.Id);
  for (uint32_t i = 0;
       render_command* command = GetNextCommand(context.Commands, i++);) {

    switch (command->Type) {
      case RENDER_COMMAND_TYPE_TEXT:
        RenderText(command->Text, windowInfo, context);
        break;

      default:
        break;
    }
  }

#if HOKI_DEV
  // DEBUG PASS
  glUseProgram(context.SimpleShader.Id);
  glDisable(GL_DEPTH_TEST);
  for (uint32_t i = 0;
       render_command* command = GetNextCommand(context.Commands, i++);) {
    switch (command->Type) {
      case DEBUG_RENDER_COMMAND_TYPE_CYCLE_SHADERS:
        HOKI_ASSERT(false);
        break;
      case DEBUG_RENDER_COMMAND_TYPE_BONES:
        DEBUG_RenderBones(*command->Entity, context);
        break;
      case DEBUG_RENDER_COMMAND_TYPE_PHYSICS_BODY:
        DEBUG_RenderPhysicsBody(*command->Body, context);
        break;
      case DEBUG_RENDER_COMMAND_TYPE_LINE:
        DEBUG_RenderLine(*command->Line, context);
        break;
      case DEBUG_RENDER_COMMAND_TYPE_SET_WIREFRAME:
        SetWireframe();
        break;
      case DEBUG_RENDER_COMMAND_TYPE_LIGHT:
        DEBUG_RenderLight(*command->Light, context);
        break;
      case DEBUG_RENDER_COMMAND_TYPE_CUBE:
        DEBUG_RenderCube(*command->CubePos, context);
        break;

      default:
        break;
    }
  }
  glEnable(GL_DEPTH_TEST);
#endif
  glDisable(GL_BLEND);

  HOKI_ASSERT_NO_OPENGL_ERRORS();
}
