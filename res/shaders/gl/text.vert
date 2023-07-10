#version 330 core
layout (location = 0) in vec2 vertex; // <vec2 pos, vec2 tex>
layout (location = 1) in vec2 texCoords; // <vec2 pos, vec2 tex>

out vec2 TexCoords;

uniform mat4 uProjection;
uniform mat4 uModelMatrix;

void main()
{
    gl_Position = uProjection * uModelMatrix * vec4(vertex, 0.0, 1.0);
    TexCoords = texCoords;
}
