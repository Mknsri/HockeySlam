#version 330 core

in vec2 TexCoords;

uniform vec3 uObjectColor;
uniform float uFill;
uniform sampler2D image_texture;
uniform sampler2D fill_texture;

out vec4 FragColor;

void main()
{
    vec4 texel = texture(image_texture, TexCoords);
    vec4 revealTexel = texture(fill_texture, TexCoords);
    float reveal = revealTexel.r;
    if (uFill * revealTexel.a < reveal) {
        discard;
    }

    FragColor = texel;
}
