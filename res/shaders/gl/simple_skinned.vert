#version 330 core

const int MAX_WEIGHTS = 4;
const int MAX_BONES = 60;

layout (location = 0) in vec3 aPos;
layout (location = 4) in ivec4 aBoneIds;
layout (location = 5) in vec4 aBoneWeights;

uniform bool uHasBones;
uniform mat4 uModelMatrix;
uniform mat4 uViewProjection;
uniform mat4 uBones[MAX_BONES];

void main()
{
    mat4 totalBoneTransform = mat4(0.0);
    vec4 totalLocalPos = vec4(aPos, 1.0);
    if (uHasBones) {
        for (int i = 0; i < MAX_WEIGHTS; i++) {
            mat4 boneTransform = uBones[aBoneIds[i]] * aBoneWeights[i];
            totalBoneTransform += boneTransform;
        }
        totalLocalPos = totalBoneTransform * totalLocalPos;
    }

    gl_Position = uViewProjection * uModelMatrix * totalLocalPos;
}
