#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 2) in vec2 aTexCoord;
layout(location = 5) in ivec4 BoneIDs;
layout(location = 6) in vec4 Weights;

const int MAX_BONES = 100;
uniform mat4 gBones[MAX_BONES];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 lightSpaceMatrix;

//out vec3 ourColor;
out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;
out vec4 FragPosLightSpace;
// flat out ivec4 Bones;
// out vec4 W;

uniform bool animated;

void main() {
	if(animated) {
		mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
		BoneTransform += gBones[BoneIDs[1]] * Weights[1];
		BoneTransform += gBones[BoneIDs[2]] * Weights[2];
		BoneTransform += gBones[BoneIDs[3]] * Weights[3];

		if(Weights[0] > 0) {
			gl_Position = projection * view * model * BoneTransform * vec4(aPos, 1.0);
		} else {
			gl_Position = projection * view * model * vec4(aPos, 1.0);
		}
	} else {
		gl_Position = projection * view * model * vec4(aPos, 1.0);
	}
    // Bones = BoneIDs;
	// W = Weights;

    //gl_Position = projection*view*model*vec4(aPos, 1.0);
    //ourColor = aColor;

	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNorm;
	TexCoord = aTexCoord;
	FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
}