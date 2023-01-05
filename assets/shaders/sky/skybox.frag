#version 450 core

layout (location = 0) out vec4 gAlbedo;
uniform samplerCube cubeMap;

in vec3 texCoords;

void main() {
	vec4 fragColor = texture(cubeMap, texCoords);
	gAlbedo = fragColor;
	// gl_FragDepth = gl_FragCoord.z;
}