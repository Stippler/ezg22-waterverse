#version 450 core

in vec3 fragPos;
in vec3 fragNormal;
in vec3 cubeCoord;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;

uniform vec3 viewPos;
uniform samplerCube cubeMap;

void main() {
    gPosition = vec4(fragPos, 1);
    gNormal = vec4(normalize(fragNormal), 1);

    vec3 I = normalize(gPosition.xyz - viewPos);
    vec3 R = reflect(I, gNormal.xyz);

    vec4 color = texture(cubeMap, R); // vec3(.4,.8,1);
    gAlbedo = vec4(vec3(color), 1);
}