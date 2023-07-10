#include "asset_model.h"

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#define STBI_NO_STDIO
#define TINYGLTF_NOEXCEPTION // optional. disable exception handling.

#include "../../3rdparty/tiny_gltf.h"

namespace Asset {

struct buffer_walker
{
  const char* Name;
  uint8_t* Cursor;
  size_t Count;
  uint8_t DataSize;
  uint8_t TargetDataSize;
  bool PackingAllowed;
};

buffer_walker get_buffer_walker(const tinygltf::Model& model,
                                const char* accessorName,
                                const tinygltf::Accessor& accessor,
                                const uint8_t targetDataSize)
{
  const int bufferViewIndex = accessor.bufferView;
  const tinygltf::BufferView& bufferView =
    model.bufferViews.at(bufferViewIndex);
  const int bufferIndex = bufferView.buffer;
  const tinygltf::Buffer& buffer = model.buffers.at(bufferIndex);

  buffer_walker result = { accessorName };
  result.Cursor = (uint8_t*)buffer.data.data() + bufferView.byteOffset;
  result.Count = accessor.count;
  result.DataSize =
    (uint8_t)(tinygltf::GetComponentSizeInBytes(accessor.componentType) *
              tinygltf::GetNumComponentsInType(accessor.type));
  result.TargetDataSize = targetDataSize;

  HOKI_WARN_MESSAGE(targetDataSize >= result.DataSize,
                    "Data size (%d) does not fit size given by GLTF (%d), "
                    "packing is used and may lead to models not working!",
                    targetDataSize,
                    result.DataSize);

  // Yikes
  // hoki has fixed types for different primitive values but gltf does not
  // we can pack everything in most cases since things like indices
  // rarely need 32-bits, but fits in 16-bits instead.
  // For safety we check PackingAllowed if we can get away with it
  // TANGENT is v4 in GTLF for checking handedness but we don't need that so
  // packing actually just copies the v3 from the v4 :[]
  result.PackingAllowed =
    (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT &&
     accessor.componentType != TINYGLTF_COMPONENT_TYPE_DOUBLE &&
     accessor.componentType != -1) ||
    strcmp(accessorName, "TANGENT") == 0;
  HOKI_ASSERT(targetDataSize >= result.DataSize || result.PackingAllowed);

  return result;
}

buffer_walker get_buffer_walker(const tinygltf::Model& model,
                                const tinygltf::Primitive& primitive,
                                const std::string accessorName,
                                const uint8_t targetDataSize)
{
  if (primitive.attributes.find(accessorName) == primitive.attributes.end()) {
    buffer_walker empty = { "EMPTY" };
    return empty;
  }

  const int accessorIndex = primitive.attributes.at(accessorName);
  const tinygltf::Accessor& accessor = model.accessors.at(accessorIndex);
  return get_buffer_walker(
    model, accessorName.c_str(), accessor, targetDataSize);
}

void populate_transform_components(const tinygltf::Node& node,
                                   mat4x4& outTransform,
                                   v3& outTranslation,
                                   quat& outRotation,
                                   v3& outScale)
{
  outTranslation = V3_ZERO;
  outRotation = IDENTITY_ROTATION;
  outScale = _v3(1.0f);

  if (node.matrix.size() == 16) {
    for (size_t i = 0; i < 16; i++) {
      outTransform.S[i] = (float)node.matrix[i];
    }
  } else {
    if (node.translation.size() == 3) {
      outTranslation.X = (float)node.translation[0];
      outTranslation.Y = (float)node.translation[1];
      outTranslation.Z = (float)node.translation[2];
    }

    if (node.rotation.size() == 4) {
      outRotation.X = (float)node.rotation[0];
      outRotation.Y = (float)node.rotation[1];
      outRotation.Z = (float)node.rotation[2];
      outRotation.W = (float)node.rotation[3];
    }

    if (node.scale.size() == 3) {
      outScale.X = (float)node.scale[0];
      outScale.Y = (float)node.scale[1];
      outScale.Z = (float)node.scale[2];
    }

    outTransform = mat4x4_translate(outTranslation) *
                   quat_to_mat4x4(outRotation) * mat4x4_scale(outScale);
  }
}

void load_primitive_data_part(uint8_t* data, buffer_walker& walker)
{
  if (walker.Count == 0) {
    return;
  }

  size_t stepSize = 1;
  if (walker.DataSize < walker.TargetDataSize) {
    stepSize = walker.TargetDataSize / walker.DataSize;
  }

  for (size_t i = 0; i < walker.DataSize; i++) {
    *data = *walker.Cursor++;
    data += stepSize;
  }

  if (walker.TargetDataSize < walker.DataSize) {
    HOKI_ASSERT(walker.PackingAllowed);
    // Align cursor at next boundary
    walker.Cursor += (walker.DataSize - walker.TargetDataSize);
  }
}

void load_primitive_data(const tinygltf::Model& model,
                         const tinygltf::Primitive& loadedPrimitive,
                         primitive_data* primitiveDataCursor,
                         uint16_t& countLoaded)
{
  // Vertices
  HOKI_ASSERT(loadedPrimitive.attributes.find("POSITION") !=
              loadedPrimitive.attributes.end());

  buffer_walker positionWalker =
    get_buffer_walker(model, loadedPrimitive, "POSITION", sizeof(v3));
  buffer_walker normalWalker =
    get_buffer_walker(model, loadedPrimitive, "NORMAL", sizeof(v3));
  buffer_walker tangentWalker =
    get_buffer_walker(model, loadedPrimitive, "TANGENT", sizeof(v3));
  buffer_walker texCoordWalker =
    get_buffer_walker(model, loadedPrimitive, "TEXCOORD_0", sizeof(v2));
  buffer_walker jointsWalker =
    get_buffer_walker(model,
                      loadedPrimitive,
                      "JOINTS_0",
                      sizeof(uint16_t) * MAX_BONES_PER_VERTEX);
  buffer_walker weightsWalker = get_buffer_walker(
    model, loadedPrimitive, "WEIGHTS_0", sizeof(float) * MAX_BONES_PER_VERTEX);

  HOKI_ASSERT(loadedPrimitive.attributes.find("POSITION") !=
              loadedPrimitive.attributes.end());
  HOKI_ASSERT(normalWalker.Count == texCoordWalker.Count);
  HOKI_ASSERT(jointsWalker.Count == weightsWalker.Count);

  for (size_t i = 0; i < positionWalker.Count; i++) {
    *primitiveDataCursor = {};
    // Vertex
    load_primitive_data_part((uint8_t*)&primitiveDataCursor->Position,
                             positionWalker);

    // Normals
    load_primitive_data_part((uint8_t*)&primitiveDataCursor->Normal,
                             normalWalker);

    // Tangent
    load_primitive_data_part((uint8_t*)&primitiveDataCursor->Tangent,
                             tangentWalker);

    // Texture coordinates
    load_primitive_data_part((uint8_t*)&primitiveDataCursor->TextureCoord,
                             texCoordWalker);

    // Joints
    load_primitive_data_part((uint8_t*)&primitiveDataCursor->BoneIds,
                             jointsWalker);

    // Weights
    load_primitive_data_part((uint8_t*)&primitiveDataCursor->BoneWeights,
                             weightsWalker);

    primitiveDataCursor++;
    countLoaded++;
  }
}

void load_indices(const tinygltf::Model& model,
                  const tinygltf::Primitive& loadedPrimitive,
                  const uint16_t indexOffset,
                  uint16_t* indicesCursor,
                  size_t& bytesLoaded,
                  uint16_t& indexCount)
{
  // Indices
  buffer_walker indexWalker =
    get_buffer_walker(model,
                      "INDICES_0",
                      model.accessors[loadedPrimitive.indices],
                      sizeof(uint16_t));
  HOKI_ASSERT(indexWalker.Count <= UINT16_MAX);
  indexCount = (uint16_t)indexWalker.Count;
  for (size_t i = 0; i < indexCount; i++) {
    load_primitive_data_part((uint8_t*)indicesCursor, indexWalker);

    indicesCursor[0] += indexOffset;
    indicesCursor++;
  }
  bytesLoaded = sizeof(indicesCursor[0]) * indexCount;
}

model_primitive load_primitive(const tinygltf::Model& model,
                               const tinygltf::Primitive& loadedPrimitive,
                               const size_t indexOffsetBytes,
                               const size_t indexCount,
                               const texture* const textures,
                               const material* const materials)
{
  model_primitive newPrimitive = {};

  newPrimitive.IndexOffsetBytes = indexOffsetBytes;
  newPrimitive.IndexCount = indexCount;

  HOKI_ASSERT(loadedPrimitive.material > -1);
  tinygltf::Material loadedMaterial = model.materials[loadedPrimitive.material];
  tinygltf::PbrMetallicRoughness pbr = loadedMaterial.pbrMetallicRoughness;
  newPrimitive.Material = (material*)&materials[loadedPrimitive.material];

  tinygltf::TextureInfo textureInfo = pbr.baseColorTexture;
  if (textureInfo.index > -1) {
    newPrimitive.Texture = (texture*)textures + textureInfo.index;
  }

  return newPrimitive;
}

model_mesh* load_mesh_structure(const tinygltf::Model& model,
                                const size_t meshCount,
                                const texture* const textures,
                                const material* const materials,
                                primitive_data* primitiveDataCursor,
                                uint16_t* indicesCursor)
{
  model_mesh* result = (model_mesh*)allocate_t(meshCount * sizeof(model_mesh));
  *result = {};

  // Primitives are indexed from 0, but we use a contigous array for all info so
  // indices need to be offset
  uint16_t indexOffset = 0;
  size_t indexOffsetBytes = 0;
  for (size_t m = 0; m < meshCount; m++) {
    tinygltf::Mesh loadedMesh = model.meshes[m];
    model_mesh& newMesh = result[m];
    newMesh = {};
    newMesh.Name = loadedMesh.name.c_str();

    newMesh.PrimitiveCount = loadedMesh.primitives.size();
    newMesh.Primitives = (model_primitive*)allocate_t(newMesh.PrimitiveCount *
                                                      sizeof(model_primitive));
    for (size_t p = 0; p < newMesh.PrimitiveCount; p++) {
      tinygltf::Primitive loadedPrimitive = loadedMesh.primitives[p];

      uint16_t loadedPrimitiveDataCount = 0;
      uint16_t loadedIndexCount = 0;
      size_t loadedIndicesBytes = 0;

      load_primitive_data(
        model, loadedPrimitive, primitiveDataCursor, loadedPrimitiveDataCount);
      load_indices(model,
                   loadedPrimitive,
                   indexOffset,
                   indicesCursor,
                   loadedIndicesBytes,
                   loadedIndexCount);
      newMesh.Primitives[p] = load_primitive(model,
                                             loadedPrimitive,
                                             indexOffsetBytes,
                                             loadedIndexCount,
                                             textures,
                                             materials);

      primitiveDataCursor += loadedPrimitiveDataCount;
      indicesCursor += loadedIndexCount;
      indexOffset += loadedPrimitiveDataCount;
      indexOffsetBytes += loadedIndicesBytes;
    }
  }

  return result;
}

model_node* load_nodes(const tinygltf::Model& model,
                       const size_t nodeCount,
                       const model_mesh* const meshes)
{
  model_node* result = (model_node*)allocate_t(sizeof(model_node) * nodeCount);

  for (size_t n = 0; n < nodeCount; n++) {
    tinygltf::Node loadedNode = model.nodes[n];

    model_node& newNode = result[n];
    newNode = {};
    newNode.Mesh =
      loadedNode.mesh > -1 ? (model_mesh*)&meshes[loadedNode.mesh] : nullptr;
    newNode.Skinned = loadedNode.skin > -1;
    newNode.Name = to_string(loadedNode.name.c_str());
    newNode.BoneId = -1;

    if (!newNode.Skinned) {
      populate_transform_components(loadedNode,
                                    newNode.Transform,
                                    newNode.Translation,
                                    newNode.Rotation,
                                    newNode.Scale);
    } else {
      newNode.Transform = IDENTITY_MATRIX;
    }
  }

  // Set parents
  for (size_t n = 0; n < nodeCount; n++) {
    for (size_t c = 0; c < model.nodes[n].children.size(); c++) {
      int childNodeIndex = model.nodes[n].children[c];
      result[childNodeIndex].Parent = &result[n];
    }
  }

  return result;
}

texture* load_textures(tinygltf::Model& model,
                       size_t textureCount,
                       game_memory& gameMemory,
                       std::string path)
{
  texture* result = (texture*)allocate_t(textureCount * sizeof(texture));

  for (size_t i = 0; i < textureCount; i++) {
    tinygltf::Texture loadedTexture = model.textures[i];
    tinygltf::Image loadedImage = model.images[loadedTexture.source];

    texture& newTexture = result[i];
    newTexture.WrapType = TEXTURE_WRAP_REPEAT;
    if (loadedTexture.sampler > -1) {
      switch (model.samplers[loadedTexture.sampler].wrapS) {
        case TINYGLTF_TEXTURE_WRAP_REPEAT:
          newTexture.WrapType = TEXTURE_WRAP_REPEAT;
          break;

        case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
          newTexture.WrapType = TEXTURE_WRAP_CLAMP;
          break;

        default:
          newTexture.WrapType = TEXTURE_WRAP_REPEAT;
          break;
      }
    }

    if (strcmp(loadedImage.mimeType.c_str(), "") == 0) {
      newTexture = loadTexture(loadedImage.uri.c_str(), gameMemory);
    } else {
      newTexture.Name = to_string(loadedImage.name);
      newTexture.Type = TEXTURE_TYPE_IMAGE;
      newTexture.Image = (image_texture*)allocate_t(sizeof(image_texture));
      newTexture.Image->Memory =
        allocate_t(loadedImage.image.size() * sizeof(loadedImage.image[0]));
      std::copy(loadedImage.image.begin(),
                loadedImage.image.end(),
                (uint8_t*)newTexture.Image->Memory);
      newTexture.Image->Path = to_string(path);

      if (!newTexture.Image->Memory) {
        std::cout << "Texture failed to load, type : " << loadedImage.mimeType
                  << ": " << std::endl;
        HOKI_ASSERT(false);
      }
      newTexture.Image->PixelSize.X = (float)loadedImage.width;
      newTexture.Image->PixelSize.Y = (float)loadedImage.height;
      newTexture.Image->Components = loadedImage.component;
      newTexture.Image->MaterialType = to_string("texture_diffuse");
    }
  }

  return result;
}

material* loadMaterials(tinygltf::Model& model,
                        size_t materialCount,
                        const texture* const textures,
                        game_memory& memory)
{
  material* result = (material*)allocate_t(sizeof(material) * materialCount);
  material defaultMaterial = create_default_material();
  result[materialCount - 1] = defaultMaterial;

  // indices 0-(n-1) -> gltf, n = default
  for (size_t i = 0; i < (materialCount - 1); i++) {
    tinygltf::Material loadedMaterial = model.materials[i];
    tinygltf::PbrMetallicRoughness pbrMetallicRoughness =
      loadedMaterial.pbrMetallicRoughness;
    material& newMaterial = result[i];

    newMaterial = defaultMaterial;

    for (int j = 0; j < 3; j++) {
      newMaterial.Albedo.E[j] = (float)pbrMetallicRoughness.baseColorFactor[j];
    }

    newMaterial.Translucent =
      loadedMaterial.alphaMode == "BLEND" || loadedMaterial.alphaMode == "MASK";

    if (pbrMetallicRoughness.baseColorTexture.index != -1) {
      newMaterial.AlbedoMap =
        (texture*)&textures[pbrMetallicRoughness.baseColorTexture.index];
    }
    if (loadedMaterial.normalTexture.index != -1) {
      newMaterial.NormalMap =
        (texture*)&textures[loadedMaterial.normalTexture.index];
    }

    newMaterial.Roughness = (float)pbrMetallicRoughness.roughnessFactor;
    newMaterial.Metallic = (float)pbrMetallicRoughness.metallicFactor;
    if (pbrMetallicRoughness.metallicRoughnessTexture.index != -1) {
      if (pbrMetallicRoughness.roughnessFactor != 0) {
        newMaterial.RoughnessMap =
          (texture*)&textures[pbrMetallicRoughness.metallicRoughnessTexture
                                .index];
      }
      if (pbrMetallicRoughness.metallicFactor != 0) {
        newMaterial.MetallicMap =
          (texture*)&textures[pbrMetallicRoughness.metallicRoughnessTexture
                                .index];
      }
    }
  }

  return result;
}

// Only supports one skin for now
model_bone* load_bones(tinygltf::Model& model,
                       size_t boneCount,
                       model_node* const nodes)
{
  if (boneCount == 0) {
    return NULL;
  }

  model_bone* result = (model_bone*)allocate_t(sizeof(model_bone) * boneCount);
  *result = {};
  tinygltf::Skin skin = model.skins[0];

  tinygltf::Accessor& boneOffsetAccessor =
    model.accessors[skin.inverseBindMatrices];
  tinygltf::BufferView& boneOffsetBufferView =
    model.bufferViews[boneOffsetAccessor.bufferView];
  tinygltf::Buffer& boneOffsetBuffer =
    model.buffers[boneOffsetBufferView.buffer];
  mat4x4* boneOffsetCursor =
    (mat4x4*)(&boneOffsetBuffer.data[0] + boneOffsetBufferView.byteOffset);

  for (size_t b = 0; b < boneCount; b++) {
    model_bone& newBone = result[b];

    int nodeIndex = skin.joints[b];
    tinygltf::Node jointNode = model.nodes[nodeIndex];

    newBone.Name = to_string(jointNode.name);
    newBone.NodeId = nodeIndex;
    nodes[nodeIndex].BoneId = (int32_t)b;

    populate_transform_components(jointNode,
                                  newBone.Transform,
                                  newBone.Translation,
                                  newBone.Rotation,
                                  newBone.Scale);

    newBone.InverseBind = boneOffsetCursor[b];
  }

  return result;
}

animation* load_animations(tinygltf::Model& loadedModel,
                           model& resultModel,
                           size_t animationCount)
{
  animation* resultAnimations =
    (animation*)allocate_t(sizeof(animation) * animationCount);

  for (size_t i = 0; i < animationCount; i++) {
    tinygltf::Animation loadedAnimation = loadedModel.animations[i];
    animation& newAnimation = resultAnimations[i];
    newAnimation = {};

    newAnimation.Name = to_string(loadedAnimation.name);
    newAnimation.ChannelCount = (uint32_t)loadedAnimation.channels.size();
    newAnimation.Channels = (animation_channel*)allocate_t(
      sizeof(animation_channel) * newAnimation.ChannelCount);
    for (size_t c = 0; c < loadedAnimation.channels.size(); c++) {
      tinygltf::AnimationChannel loadedChannel = loadedAnimation.channels[c];
      animation_channel& newChannel = newAnimation.Channels[c];
      newChannel.BoneIndex = 0xFFFFFFFF;

      for (uint32_t b = 0; b < resultModel.BoneCount; b++) {
        if (resultModel.Bones[b].NodeId ==
            (uint32_t)loadedChannel.target_node) {
          newChannel.BoneIndex = b;
        }
      }

      // TODO: Check if skipping non-bone animation channels
      // actually works?
      if (newChannel.BoneIndex == 0xFFFFFFFF) {
        newChannel.KeyframeCount = 0;
        continue;
      }

      tinygltf::AnimationSampler& samplerForChannel =
        loadedAnimation.samplers[loadedChannel.sampler];
      tinygltf::Accessor& keyframeTimesAccessor =
        loadedModel.accessors[samplerForChannel.input];
      tinygltf::BufferView& keyframeTimesBufferView =
        loadedModel.bufferViews[keyframeTimesAccessor.bufferView];
      tinygltf::Buffer& keyframeTimesBuffer =
        loadedModel.buffers[keyframeTimesBufferView.buffer];
      float* keyframeTimeCursor =
        (float*)&keyframeTimesBuffer.data[keyframeTimesBufferView.byteOffset];

      tinygltf::Accessor& keyframeValuesAccessor =
        loadedModel.accessors[samplerForChannel.output];
      tinygltf::BufferView& keyframeValuesBufferView =
        loadedModel.bufferViews[keyframeValuesAccessor.bufferView];
      tinygltf::Buffer& keyframeValuesBuffer =
        loadedModel.buffers[keyframeValuesBufferView.buffer];
      uint8_t* keyframeValueCursor =
        &keyframeValuesBuffer.data[keyframeValuesBufferView.byteOffset];

      animation_path_type pathType = TRANSFORMATION;
      if (strcmp(loadedChannel.target_path.c_str(), "translation") == 0) {
        pathType = TRANSLATION;
      } else if (strcmp(loadedChannel.target_path.c_str(), "rotation") == 0) {
        pathType = ROTATION;
      } else if (strcmp(loadedChannel.target_path.c_str(), "scale") == 0) {
        pathType = SCALE;
      }

      bool cubic =
        strcmp(samplerForChannel.interpolation.c_str(), "CUBICSPLINE") == 0;
      newChannel.PathType = pathType;
      newChannel.KeyframeCount = keyframeTimesAccessor.count;
      newChannel.Keyframes = (animation_keyframe*)allocate_t(
        sizeof(animation_keyframe) * newChannel.KeyframeCount);

      animation_keyframe previousKeyframe = newChannel.Keyframes[0];
      float delta = 0.0f; // If channel is not animated (delta > 0), discard it
      for (size_t k = 0; k < newChannel.KeyframeCount; k++) {
        animation_keyframe& newKeyframe = newChannel.Keyframes[k];
        newKeyframe.StartTime = *(keyframeTimeCursor++);

        if (newAnimation.Duration < newKeyframe.StartTime) {
          newAnimation.Duration = newKeyframe.StartTime;
        }

        if (newChannel.Duration < newKeyframe.StartTime) {
          newChannel.Duration = newKeyframe.StartTime;
        }

        switch (pathType) {
          case TRANSLATION: {
            v3* translationCursor = (v3*)keyframeValueCursor;
            if (cubic) {
              translationCursor++;
            }
            newKeyframe.Translation = *(translationCursor++);

            if (cubic) {
              translationCursor++;
            }
            keyframeValueCursor = (uint8_t*)translationCursor;

            float prevCombined = previousKeyframe.Translation.X +
                                 previousKeyframe.Translation.Y +
                                 previousKeyframe.Translation.Z;
            float combined = newKeyframe.Translation.X +
                             newKeyframe.Translation.Y +
                             newKeyframe.Translation.Z;
            float thisDelta = abs_f(prevCombined - combined);
            if (thisDelta > delta) {
              delta = thisDelta;
            }
          } break;

          case ROTATION: {
            quat* rotationCursor = (quat*)keyframeValueCursor;

            if (cubic) {
              rotationCursor++;
            }
            // buffer is xyzw, so transform it into wxyz
            quat rotationFromBuffer = *(rotationCursor++);
            newKeyframe.Rotation.W = rotationFromBuffer.Z;
            newKeyframe.Rotation.X = rotationFromBuffer.W;
            newKeyframe.Rotation.Y = rotationFromBuffer.X;
            newKeyframe.Rotation.Z = rotationFromBuffer.Y;

            float prevCombined =
              previousKeyframe.Rotation.W + previousKeyframe.Rotation.X +
              previousKeyframe.Rotation.Y + previousKeyframe.Rotation.Z;
            float combined = newKeyframe.Rotation.W + newKeyframe.Rotation.X +
                             newKeyframe.Rotation.Y + newKeyframe.Rotation.Z;
            float thisDelta = abs_f(prevCombined - combined);
            if (thisDelta > delta) {
              delta = thisDelta;
            }

            if (cubic) {
              rotationCursor++;
            }
            keyframeValueCursor = (uint8_t*)rotationCursor;
          } break;

          case SCALE: {
            v3* scaleCursor = (v3*)keyframeValueCursor;
            if (cubic) {
              scaleCursor++;
            }
            newKeyframe.Scale = *(scaleCursor++);

            float prevCombined = previousKeyframe.Scale.X +
                                 previousKeyframe.Scale.Y +
                                 previousKeyframe.Scale.Z;
            float combined =
              newKeyframe.Scale.X + newKeyframe.Scale.Y + newKeyframe.Scale.Z;
            float thisDelta = abs_f(prevCombined - combined);
            if (thisDelta > delta) {
              delta = thisDelta;
            }

            if (cubic) {
              scaleCursor++;
            }
            keyframeValueCursor = (uint8_t*)scaleCursor;
          } break;

          default:
            HOKI_ASSERT(false);
            break;
        }

        previousKeyframe = newKeyframe;
      }
#if 0
                if (delta < 0.0001f) {
                    newChannel.KeyframeCount = 0;
                }
#endif
    }
  }

  return resultAnimations;
}

bool tinyGltf_fileExists(const std::string& abs_filename, void* user_data)
{
  return ((game_memory*)user_data)->GetFileSize(abs_filename.c_str()) > 0;
}

bool tinyGltf_readFile(std::vector<unsigned char>* out,
                       std::string* err,
                       const std::string& filepath,
                       void* user_data)
{
  size_t filesize = ((game_memory*)user_data)->GetFileSize(filepath.c_str());
  uint8_t* filePtr = (uint8_t*)allocate_t(filesize);
  ((game_memory*)user_data)->ReadFile(filepath.c_str(), filePtr);
  out->assign(filePtr, filePtr + filesize);

  return true;
}

std::string tinyGltf_expandFilePath(const std::string& path, void* user_data)
{
  return path;
}

model loadModel(const char* path, game_memory& gameMemory)
{
  model result = {};

  tinygltf::Model model;
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  tinygltf::FsCallbacks callbacks = {};
  callbacks.FileExists = &tinyGltf_fileExists;
  callbacks.ExpandFilePath = &tinyGltf_expandFilePath;
  callbacks.ReadWholeFile = &tinyGltf_readFile;
  callbacks.user_data = &gameMemory;

  loader.SetFsCallbacks(callbacks);

  const file loadedFile = load_file(path, gameMemory);

  bool ret = false;
  if (std::strcmp(loadedFile.FileExt.Value, "gltf") == 0) {
    ret = loader.LoadASCIIFromString(&model,
                                     &err,
                                     &warn,
                                     (const char*)loadedFile.Memory,
                                     (unsigned int)loadedFile.Size,
                                     loadedFile.Directory.Value);
  } else {
    ret = loader.LoadBinaryFromMemory(&model,
                                      &err,
                                      &warn,
                                      (const unsigned char*)loadedFile.Memory,
                                      (unsigned int)loadedFile.Size,
                                      path,
                                      0);
  }

  HOKI_ASSERT(ret);

  char buffer[256];
  if (!warn.empty()) {
    snprintf(buffer, 256, "Warn: %s\n", warn.c_str());
    HOKI_ASSERT(false);
  }

  if (!err.empty()) {
    snprintf(buffer, 256, "Err: %s\n", err.c_str());
    HOKI_ASSERT(false);
  }

  if (!ret) {
    snprintf(buffer, 256, "Failed to parse glTF\n");
    HOKI_ASSERT(false);
  }

  result.Name = loadedFile.FileName.Value;

  result.TextureCount = model.textures.size();
  result.Textures = load_textures(model, result.TextureCount, gameMemory, path);

  result.MaterialCount =
    model.materials.size() > 0 ? model.materials.size() + 1 : 1;
  result.Materials =
    loadMaterials(model, result.MaterialCount, result.Textures, gameMemory);

  // Allocated a single array for all primitive data
  result.MeshCount = model.meshes.size();
  for (size_t m = 0; m < result.MeshCount; m++) {
    tinygltf::Mesh loadedMesh = model.meshes[m];

    for (size_t p = 0; p < loadedMesh.primitives.size(); p++) {
      tinygltf::Primitive loadedPrimitive = loadedMesh.primitives[p];
      buffer_walker positionWalker =
        get_buffer_walker(model, loadedPrimitive, "POSITION", UINT8_MAX);
      result.VertexCount += positionWalker.Count;

      tinygltf::Accessor indicesAccessor =
        model.accessors[loadedPrimitive.indices];
      result.IndexCount += indicesAccessor.count;
    }
  }
  result.Vertices =
    (primitive_data*)allocate_t(sizeof(primitive_data) * result.VertexCount);
  result.Indices = (uint16_t*)allocate_t(sizeof(uint16_t) * result.IndexCount);

  result.Meshes = load_mesh_structure(model,
                                      result.MeshCount,
                                      result.Textures,
                                      result.Materials,
                                      result.Vertices,
                                      result.Indices);

  result.NodeCount = model.nodes.size();
  result.Nodes = load_nodes(model, result.NodeCount, result.Meshes);

  result.BoneCount = model.skins.size() > 0 ? model.skins[0].joints.size() : 0;
  result.Bones = load_bones(model, result.BoneCount, result.Nodes);

  result.GlobalInverseTransform = IDENTITY_MATRIX;
  if (result.BoneCount > 0) {
    tinygltf::Skin skin = model.skins[0];
    tinygltf::Node* skeletonRootNode = nullptr;
    if (skin.skeleton != -1) {
      skeletonRootNode = &model.nodes[skin.skeleton];
    }
    for (size_t i = 0; i < model.nodes.size(); i++) {
      if (model.nodes[i].skin > -1 && model.nodes[i].mesh > -1) {
        skeletonRootNode = &model.nodes[i];
        break;
      }
    }
    if (skeletonRootNode != nullptr) {
      mat4x4 globalTransform = IDENTITY_MATRIX;

      if (skeletonRootNode->matrix.size() == 16) {
        for (size_t i = 0; i < 16; i++) {
          globalTransform.S[i] = (float)skeletonRootNode->matrix[i];
        }
      } else {
        v3 translation = V3_ZERO;
        if (skeletonRootNode->translation.size() == 3) {
          translation.X = (float)skeletonRootNode->translation[0];
          translation.Y = (float)skeletonRootNode->translation[1];
          translation.Z = (float)skeletonRootNode->translation[2];
        }

        quat rotation = IDENTITY_ROTATION;
        if (skeletonRootNode->rotation.size() == 4) {
          rotation.X = (float)skeletonRootNode->rotation[0];
          rotation.Y = (float)skeletonRootNode->rotation[1];
          rotation.Z = (float)skeletonRootNode->rotation[2];
          rotation.W = (float)skeletonRootNode->rotation[3];
        }

        v3 scale = _v3(1.0f);
        if (skeletonRootNode->scale.size() == 3) {
          scale.X = (float)skeletonRootNode->scale[0];
          scale.Y = (float)skeletonRootNode->scale[1];
          scale.Z = (float)skeletonRootNode->scale[2];
        }

        globalTransform = mat4x4_translate(translation) *
                          quat_to_mat4x4(rotation) * mat4x4_scale(scale);
      }

      result.GlobalInverseTransform = mat4x4_invert(globalTransform);
    }
  }

  result.AnimationCount = model.animations.size();
  result.Animations = load_animations(model, result, result.AnimationCount);

  return result;
}
}