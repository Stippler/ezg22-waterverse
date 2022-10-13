#version 450 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;

out vec2 fragTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

mat4 buildTranslation(vec3 delta) {
    return mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(delta, 1.0));
}

mat4 buildScaling(vec3 delta) {
    mat4 scale = mat4(1.0);
    scale[0][0] = delta[0];
    scale[1][1] = delta[1];
    scale[2][2] = delta[2];
    return scale;
}

void main() {
    mat4 trans = mat4(1.0f);

    float scale = 20;
    trans *= buildTranslation(vec3(-scale/2, -5.0, -scale/2));
    trans *= buildScaling(vec3(scale, 1.0, scale));

    gl_Position = projection * view * trans * vec4(pos, 1.0);
    // fragPos = vec3(model*vec4(pos, 1.0));
    fragTexCoord = texCoord;
}