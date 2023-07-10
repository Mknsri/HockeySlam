#version 300 es
precision highp float;

const int MAX_WEIGHTS = 4;
const int MAX_BONES = 60;

layout (location = 0) in vec3 aPos;
layout (location = 4) in ivec4 aBoneIds;
layout (location = 5) in vec4 aBoneWeights;

uniform bool uHasBones;
uniform mat4 uModelMatrix;
uniform mat4 uViewProjection;
uniform vec4 uBones[MAX_BONES * 4];
/**
* uBones will break with mat4 arrays, so we use vec4 instead
* https://stackoverflow.com/questions/20388086/opengl-es-3-0-matrix-array-only-using-first-matrix
*/
void main() {
    mat4 totalBoneTransform = mat4(0.0);
    vec4 totalLocalPos = vec4(aPos, 1.0);
    if (uHasBones) {
        for (int i = 0; i < MAX_WEIGHTS; i++) {
            int boneId = aBoneIds[i] * 4;
            mat4 boneTransform = mat4(
            uBones[boneId + 0],
            uBones[boneId + 1],
            uBones[boneId + 2],
            uBones[boneId + 3]) * aBoneWeights[i];

            totalBoneTransform += boneTransform;
        }
        totalLocalPos = totalBoneTransform * totalLocalPos;
    }

    gl_Position = uViewProjection * uModelMatrix * totalLocalPos;
}
