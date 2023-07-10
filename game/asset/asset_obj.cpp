#include "asset_obj.h"

bool hasFace(obj_face* faces, uint32_t faceCount, obj_face* face)
{
  for (uint32_t i = 0; i < faceCount; i++) {
    obj_face* existingFace = &faces[i];
    if (face->VertexIndex == existingFace->VertexIndex &&
        face->UVIndex == existingFace->UVIndex &&
        face->NormalIndex == existingFace->NormalIndex) {
      return true;
    }
  }

  return false;
}

void createOBJ(uint8_t* begin, obj_model* out)
{
  obj_model* result = out;

  char* cursor = (char*)begin;
  bool newLine = true;

  // Count data types
  while (*cursor) {
    if (string_icompare(cursor, "\n")) {
      cursor++;
      newLine = true;
      continue;
    }

    if (newLine) {
      char identifier = (char)char_tolower(*cursor++);
      switch (identifier) {
        case 'v': {
          identifier = (char)char_tolower(*(cursor++));
          switch (identifier) {
            case ' ':
              result->VertexCount += 3;
              break;
            case 't':
              result->UVCount++;
              break;
            case 'n':
              result->NormalCount++;
              break;
          }

        } break;
        case 'f':
          result->FaceCount += 3;
          break;

        case '#':
        default:
          newLine = false;
          continue;
          // HOKI_ASSERT(false);
          break;
      }
      newLine = false;
    } else {
      cursor++;
    }
  }

  // allocate_t space for all the data types
  result->Vertices = (v3*)allocate_t(result->VertexCount * sizeof(float));
  result->UVs = (v2*)allocate_t(result->UVCount * sizeof(v2));
  result->Normals = (v3*)allocate_t(result->NormalCount * sizeof(v3));
  result->Faces = (obj_face*)allocate_t(result->FaceCount * sizeof(obj_face));

  // Get the actual data
  cursor = (char*)begin;
  while (*cursor) {
    if (string_icompare(cursor, "\n")) {
      cursor++;
      newLine = true;
      continue;
    }

    obj_line_type type = UNKNOWN;
    if (newLine) {
      char identifier = (char)char_tolower(*cursor++);
      switch (identifier) {
        case 'v': {
          identifier = (char)char_tolower(*(cursor++));
          switch (identifier) {
            case ' ':
              type = VERTEX;
              break;
            case 't':
              type = TEXTURE_COORDINATE;
              break;
            case 'n':
              type = VERTEX_NORMAL;
              break;
          }

        } break;
        case 'f':
          type = FACE;
          break;

        case '#':
        default:
          newLine = false;
          continue;
          // HOKI_ASSERT(false);
          break;
      }
      newLine = false;
    } else {
      cursor++;
    }

    uint32_t filledVertices = 0, filledUVs = 0, filledNormals = 0,
             filledFaces = 0;
    switch (type) {
      case VERTEX: {
        v3* vertex = &result->Vertices[filledVertices++];
        *vertex = {};
        vertex->X = strtof(cursor, &cursor);
        vertex->Y = strtof(cursor, &cursor);
        vertex->Z = strtof(cursor, &cursor);

      } break;
      case TEXTURE_COORDINATE: {
        v2* uv = &result->UVs[filledUVs++];
        *uv = {};
        uv->U = strtof(cursor, &cursor);
        uv->V = strtof(cursor, &cursor);

      } break;
      case VERTEX_NORMAL: {

        v3* normal = &result->Normals[filledNormals++];
        *normal = {};
        normal->X = strtof(cursor, &cursor);
        normal->Y = strtof(cursor, &cursor);
        normal->Z = strtof(cursor, &cursor);
      } break;
      case FACE: {
        for (int i = 0; i < 3; i++) {
          obj_face* face = &result->Faces[filledFaces++];

          face->VertexIndex = (strtol(cursor, &cursor, 10)) - 1;
          face->UVIndex = (strtol(++cursor, &cursor, 10)) - 1;
          face->NormalIndex = (strtol(++cursor, &cursor, 10)) - 1;

          if (hasFace(result->Faces, filledFaces - 1, face)) {
            filledFaces--;
          }
        }

      } break;
    }
  }
}
