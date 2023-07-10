#include "glsl_header.h"
#if GLES
GLSL_PREPROCESS(version 300 es)
precision highp float;
#else
GLSL_PREPROCESS(version 330 core)
#endif

const int MAX_WEIGHTS = 4;
const int MAX_BONES = 60;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aTangents;
layout(location = 4) in uvec4 aBoneIds;
layout(location = 5) in vec4 aBoneWeights;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace;

uniform bool uHasBones;
uniform mat4 uModelMatrix;
uniform mat4 uViewProjection;
#ifdef GLES
/**
 * uBones will break with mat4 arrays, so we use vec4 instead
 * https://stackoverflow.com/questions/20388086/opengl-es-3-0-matrix-array-only-using-first-matrix
 */
uniform vec4 uBones[MAX_BONES * 4];
#else
uniform mat4 uBones[MAX_BONES];
#endif
uniform mat4 uLightSpaceMatrix;
#ifdef SHADER_INSTANCED
uniform int uInstanceCount;
uniform vec3 uInstanceSpacing;
#endif

#include "light_inc.glsl"
uniform DirLight uDirLight;

void main()
{
#ifdef SHADER_INSTANCED
  float instancePlusOne = float(gl_InstanceID + 1);
  float zScale = uInstanceSpacing.z;
  float xyScale = uInstanceSpacing.x;
  int cols = (uInstanceCount / 4) + 1;
  float offset = ceil(instancePlusOne / float(cols));
  float Z = offset * zScale;
  float XOFF = int(offset) % 2 == 0 ? 0.0 : (xyScale * 0.5);
  float X = (float(gl_InstanceID % cols) * xyScale) + XOFF;
  float Y = offset * uInstanceSpacing.y;

  vec4 totalLocalPos = vec4(aPos + vec3(X, Y, Z), 1.0);
#else
  vec4 totalLocalPos = vec4(aPos, 1.0);
#endif

  mat4 totalBoneTransform = mat4(0.0);
  vec4 totalLocalNormal = vec4(aNormal, 1.0);
  if (uHasBones) {
#ifdef GLES
    for (int i = 0; i < MAX_WEIGHTS; i++) {
      int boneId = int(aBoneIds[i]) * 4;
      mat4 boneTransform = mat4(uBones[boneId + 0],
                                uBones[boneId + 1],
                                uBones[boneId + 2],
                                uBones[boneId + 3]) *
                           aBoneWeights[i];

      totalBoneTransform += boneTransform;
    }
#else
    for (int i = 0; i < MAX_WEIGHTS; i++) {
      mat4 boneTransform = uBones[aBoneIds[i]] * aBoneWeights[i];
      totalBoneTransform += boneTransform;
    }
#endif
    totalLocalPos = totalBoneTransform * totalLocalPos;
    totalLocalNormal = totalBoneTransform * totalLocalNormal;
  }

  TexCoords = aTexCoords;
  vec4 fragPosW = uModelMatrix * totalLocalPos;

  gl_Position = uViewProjection * fragPosW;
  FragPos = fragPosW.xyz / fragPosW.w;
  Normal = normalize(mat3(uModelMatrix) * totalLocalNormal.xyz);

  vec3 toLight = normalize(uDirLight.Direction - FragPos);
  float cosLightAngle = 1.0 - dot(toLight, Normal);
  float normalOffsetScale = 0.005 * clamp(cosLightAngle, 0.0, 1.0);
  FragPosLightSpace =
    uLightSpaceMatrix * (fragPosW + (totalLocalNormal * normalOffsetScale));
}
