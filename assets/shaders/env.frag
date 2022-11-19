#version 450 core

layout (location = 0) out vec4 environment;

in vec4 worldPos;
in float depth;

void main() {
  environment = vec4(worldPos.xyz, depth);
  //environment = vec4(depth);
  //gl_FragColor = vec4(worldPos.xyz, depth);
}