#version 330 core
out vec4 FragColor;

in vec4 boneWeight;

void main()
{
    FragColor = boneWeight;
}
