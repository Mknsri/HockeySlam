#version 300 es
precision highp float;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

uniform mat4 uModelMatrix;
uniform mat4 uViewProjection;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;

    gl_Position = uViewProjection * uModelMatrix * vec4(aPos, 1.0);
}
