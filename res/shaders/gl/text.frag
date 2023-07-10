#version 330 core

in vec2 TexCoords;

uniform sampler2D uCharacterTexture;
uniform vec3 uTextColor;

out vec4 FragColor;

void main()
{
  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(uCharacterTexture, TexCoords).r);

  vec3 color = uTextColor;

  FragColor = vec4(color, sampled.a) * sampled;
}
