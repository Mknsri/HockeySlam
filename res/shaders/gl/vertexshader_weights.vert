#version 330 core

const int MAX_WEIGHTS = 4;
const int MAX_BONES = 60;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormals;
layout (location = 3) in vec3 aTangents;
layout (location = 4) in ivec4 aBoneIds;
layout (location = 5) in vec4 aBoneWeights;

out vec4 boneWeight;

uniform mat4 uModelMatrix;
uniform mat4 uViewProjection;
uniform mat4 uBones[MAX_BONES];

void main() {
    vec4 combinedBoneWeight = vec4(aBoneWeights.xyz, 1.0);
    vec4 totalLocalPos = vec4(0.0);
    for (int i = 0; i < MAX_WEIGHTS; i++) {
        vec4 localPos = uBones[aBoneIds[i]] * vec4(aPos, 1.0);
        totalLocalPos += localPos * aBoneWeights[i];
    }

    boneWeight = combinedBoneWeight;
    gl_Position = uViewProjection * uModelMatrix * totalLocalPos;
}
