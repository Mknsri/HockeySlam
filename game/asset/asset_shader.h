#ifndef OGL_SHADER_H
#define OGL_SHADER_H

namespace Asset {

enum ogl_shader_type
{
  SHADER_TYPE_ANIMATED_MESH,
  SHADER_TYPE_WEIGHTS,
  SHADER_TYPE_SIMPLE,
  SHADER_TYPE_SIMPLE_TEXTURED,
  SHADER_TYPE_TEXT,
  SHADER_TYPE_SHADOW,
  SHADER_TYPE_FILLED,
  SHADER_TYPE_UI,
  SHADER_TYPE_PBR,
  SHADER_TYPE_PBR_INSTANCED
};
struct ogl_shader
{
  const char* Name;
  char* Memory;
  size_t Size;
};

struct ogl_shader_program
{
  ogl_shader* VertexShader;
  ogl_shader* FragmentShader;
  ogl_shader_type Type;
};
}

#endif // OGL_SHADER_H