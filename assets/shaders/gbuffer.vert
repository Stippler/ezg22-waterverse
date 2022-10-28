#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 5) in ivec4 BoneIDs;
layout(location = 6) in vec4 Weights;

const int MAX_BONES = 100;
uniform mat4 gBones[MAX_BONES];

out vec3 FragPos;
out vec2 TexCoord;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool animated;

void main() {
    vec4 viewPos;
    if(animated) {
        mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
        BoneTransform += gBones[BoneIDs[1]] * Weights[1];
        BoneTransform += gBones[BoneIDs[2]] * Weights[2];
        BoneTransform += gBones[BoneIDs[3]] * Weights[3];
        if(Weights[0] > 0) {
            viewPos = view * model * BoneTransform * vec4(aPos, 1.0);
        } else {
            viewPos = view * model * vec4(aPos, 1.0);
        }
    } else {
        viewPos = view * model * vec4(aPos, 1.0);
    }
    FragPos = viewPos.xyz;
    TexCoord = aTexCoord;

    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    Normal = normalMatrix * aNormal;

    gl_Position = projection * viewPos;
}