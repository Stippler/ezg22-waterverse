#version 450 core

in vec3 normal;
in vec3 pos;

uniform vec3 viewPos;
uniform samplerCube cubeMap;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;

void main()
{    
    vec3 I = normalize(pos - viewPos);
    vec3 R = reflect(I, normalize(normal));

    gPosition = vec4(pos, 1);
    gNormal = vec4(normalize(normal), 1);
    vec4 color = texture(cubeMap, R);
    gAlbedo = vec4(vec3(color), 1.0);
}