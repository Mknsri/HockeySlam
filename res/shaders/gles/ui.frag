#version 300 es
precision highp float;

in vec2 TexCoords;

uniform vec3 uObjectColor;
uniform vec2 uTextureOffset;
uniform sampler2D uTexture_diffuse1;

out vec4 FragColor;

void main()
{
    vec4 texel = texture(uTexture_diffuse1, TexCoords + uTextureOffset);

    FragColor = vec4(1.0) * texel;
}
