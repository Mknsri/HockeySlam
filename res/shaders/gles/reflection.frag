#version 300 es
precision highp float;

in vec2 TexCoords;
in vec3 FragPos;

uniform vec3 uObjectColor;
uniform sampler2D texture_diffuse1;

out vec4 FragColor;

void main()
{
    vec4 texel = texture(texture_diffuse1, TexCoords);
    texel.a = 0.4 - (FragPos.y * -0.3);
    if (FragPos.y > 0)
        texel.a = 0;

    FragColor = texel;
}
