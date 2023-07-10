#ifndef OBJ_H
#define OBJ_H

enum obj_line_type
{
  VERTEX,
  TEXTURE_COORDINATE,
  VERTEX_NORMAL,
  FACE,
};

struct obj_face
{
  uint32_t VertexIndex;
  uint32_t UVIndex;
  uint32_t NormalIndex;
};

struct obj_model
{
  uint32_t VertexCount;
  uint32_t UVCount;
  uint32_t NormalCount;
  v3* Vertices;
  v2* UVs;
  v3* Normals;

  uint32_t FaceCount;
  obj_face* Faces;
};

#endif // OBJ_H