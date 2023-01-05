#version 450 core

layout(location = 0) in vec3 vPos;

uniform mat4 view;
uniform mat4 projection;

uniform vec3 viewPos;

out vec3 texCoords;

void main(){
	texCoords = vPos;
	gl_Position = projection * view * vec4(vPos, 1.0);
}