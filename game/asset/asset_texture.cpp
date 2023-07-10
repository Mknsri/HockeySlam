#include "asset_texture.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC(sz) allocate_t(sz)
#define STBI_REALLOC(p, newsz) reallocate_t((uint8_t*)p, newsz)
#define STBI_FREE(p) unallocate_t(p)
#include "../../3rdparty/stb_image.h"

#include "asset_file.h"
#include "asset_dds.h"

namespace Asset {

texture loadTexture(const uint8_t* buffer,
                    size_t size,
                    const std::string mimeType,
                    const std::string typeName,
                    game_memory& gameMemory)
{
  texture result = {};

  if (strcmp(mimeType.c_str(), "image/vnd-ms.dds") == 0) {
    result.Type = TEXTURE_TYPE_DDS;
    result.DDSImage = (DDS_Image*)allocate_t(sizeof(DDS_Image));
    *result.DDSImage = createDDS((uint8_t*)buffer, true);

  } else {
    result.Type = TEXTURE_TYPE_IMAGE;
    result.Image = (image_texture*)allocate_t(sizeof(image_texture));
    image_texture* image = result.Image;
    int width, height, components;
    image->Memory = stbi_load_from_memory(
      (stbi_uc*)buffer, (int)size, &width, &height, &components, 0);

    if (!image->Memory) {
      std::cout << "Texture failed to load, type : " << mimeType << ": "
                << stbi_failure_reason() << std::endl;
      HOKI_ASSERT(false);
    }
    image->PixelSize = _v2((float)width, (float)height);

    image->Components = components;
    image->MaterialType = to_string(typeName);
  }

  HOKI_ASSERT(result.Type != TEXTURE_TYPE_INVALID);
  return result;
}

texture loadTexture(const std::string path,
                    const std::string typeName,
                    game_memory& gameMemory)
{
  const file loadedFile = load_file(path, gameMemory);

  texture result = {};

  if (strcmp(loadedFile.FileExt.Value, "dds") == 0) {
    result.Type = TEXTURE_TYPE_DDS;
    result.DDSImage = (DDS_Image*)allocate_t(sizeof(DDS_Image));
    *result.DDSImage = createDDS((uint8_t*)loadedFile.Memory, true);

  } else {
    result.Type = TEXTURE_TYPE_IMAGE;
    result.Image = (image_texture*)allocate_t(sizeof(image_texture));
    image_texture* image = result.Image;
    int width, height, components;
    image->Memory = stbi_load_from_memory((stbi_uc*)loadedFile.Memory,
                                          (int)loadedFile.Size,
                                          &width,
                                          &height,
                                          &components,
                                          0);

    if (!image->Memory) {
      std::cout << "Texture failed to load at path: " << path << ": "
                << stbi_failure_reason() << std::endl;
      HOKI_ASSERT(false);
    }
    image->Path = to_string(path);
    image->PixelSize = _v2((float)width, (float)height);

    image->Components = components;
    image->MaterialType = to_string(typeName);
  }

  HOKI_ASSERT(result.Type != TEXTURE_TYPE_INVALID);
  return result;
}

texture loadTexture(const std::string path, game_memory& gameMemory)
{
  return loadTexture(path, "texture_diffuse", gameMemory);
}

texture loadTexture(const uint8_t* buffer,
                    size_t size,
                    const std::string mimeType,
                    game_memory& gameMemory)
{
  return loadTexture(buffer, size, mimeType, "texture_diffuse", gameMemory);
}
}