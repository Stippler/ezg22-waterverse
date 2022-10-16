#version 450 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;

out vec2 fragTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D tex;

mat4 buildTranslation(vec3 delta) {
    mat4 trans = mat4(1.0);
    trans[0][0] = 1;
    trans[1][1] = 1;
    trans[2][2] = 1;
    trans[3][3] = 1;
    trans[3][0] = delta[0];
    trans[3][1] = delta[1];
    trans[3][2] = delta[2];
    return trans;
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

    float height = texture(tex, texCoord).x;      

    trans *= buildTranslation(vec3(-scale/2, height-5, -scale/2));
    trans *= buildScaling(vec3(scale, 1, scale));

    gl_Position = projection * view * trans * vec4(pos, 1.0);
    // fragPos = vec3(model*vec4(pos, 1.0));
    fragTexCoord = texCoord;
}