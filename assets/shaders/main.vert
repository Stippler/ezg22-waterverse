#version 450 core

layout(location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertTexCoords;

out vec2 texCoords;

void main()
{
    gl_Position = vec4(vertPos.x, vertPos.y, 0.0, 1.0);
    texCoords = vertTexCoords;
}