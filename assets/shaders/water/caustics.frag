#version 450 core

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 fragNormal;
in vec3 fragPos;

uniform DirLight light;
uniform vec3 viewPos;
uniform sampler2D tex;

layout (location = 0) out vec4 fragColor;
in vec2 fragTexCoord;

float causticsFactor = 0.8;

in vec3 oldPosition;
in vec3 newPosition;
in float waterDepth;
in float depth;

void main() {
    float causticsIntensity = 0.0;

    if (depth >= waterDepth) {
        float oldArea = length(dFdx(oldPosition)) * length(dFdy(oldPosition));
        float newArea = length(dFdx(newPosition)) * length(dFdy(newPosition));

        float ratio;

        // Prevent dividing by zero (debug NVidia drivers)
        if (newArea == 0.0) {
        // Arbitrary large value
        ratio = 2.0e+20;
        } else {
        ratio = oldArea / newArea;
        }

        causticsIntensity = causticsFactor * ratio;
    }             
    
    fragColor = vec4(causticsIntensity, 0.0, 0.0, depth);
}