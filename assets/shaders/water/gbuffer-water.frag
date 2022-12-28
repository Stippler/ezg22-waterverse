#version 450 core

in vec3 fragPos;
in vec3 fragNormal;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;


void main() {             
    vec3 color = vec3(.4,.8,1);

    gPosition = vec4(fragPos, 1);
    gNormal = vec4(normalize(fragNormal), 1);
    gAlbedo = vec4(color, 1);
}