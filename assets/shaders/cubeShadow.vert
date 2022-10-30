#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 5) in ivec4 BoneIDs;
layout (location = 6) in vec4 Weights;

const int MAX_BONES = 100;
uniform mat4 gBones[MAX_BONES];

uniform mat4 model;
uniform bool animated;

void main()
{ 
    if(animated){
        mat4 BoneTransform = gBones[BoneIDs[0]]*Weights[0];
        BoneTransform += gBones[BoneIDs[1]]*Weights[1];
        BoneTransform += gBones[BoneIDs[2]]*Weights[2];
        BoneTransform += gBones[BoneIDs[3]]*Weights[3];
        if(Weights[0] > 0){
            gl_Position = model*BoneTransform*vec4(aPos, 1.0);
        }
        else{
            gl_Position = model*vec4(aPos, 1.0);
        }
    }
    else{
        gl_Position = model * vec4(aPos, 1.0);
    } 
}  