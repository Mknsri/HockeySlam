#include "asset_shader.h"

#include "asset_file.h"

namespace Asset {
ogl_shader* loadShader(std::string path, game_memory& gameMemory)
{
  ogl_shader* result = (ogl_shader*)allocate_t(sizeof(ogl_shader));

  file loadedFile = load_file(path, gameMemory);
  HOKI_ASSERT(strcmp(loadedFile.FileExt.Value, "vert") == 0 ||
              strcmp(loadedFile.FileExt.Value, "frag") == 0);

#if HOKI_DEV
  result->Name = loadedFile.Path.Value;
#endif
  // Strip whitespace
  result->Memory = (char*)loadedFile.Memory;
  while (result->Memory[0] == '\n' || result->Memory[0] == '\r') {
    result->Memory++;
  }
  result->Size = loadedFile.Size;

  return result;
}

ogl_shader_program create_shader_program(ogl_shader* vertexShader,
                                         ogl_shader* fragmentShader,
                                         ogl_shader_type type)
{
  ogl_shader_program result = {};
  result.VertexShader = vertexShader;
  result.FragmentShader = fragmentShader;
  result.Type = type;

  return result;
}
}