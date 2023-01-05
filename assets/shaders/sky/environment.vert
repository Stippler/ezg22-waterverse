#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 normal;
out vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // mat3(transpose(inverse(model))) *
    normal =  aNormal;
    pos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
