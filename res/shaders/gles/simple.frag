#version 300 es
precision highp float;

uniform vec3 uObjectColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(uObjectColor, 1.0);
}
