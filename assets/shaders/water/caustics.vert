#version 450 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;

out vec2 fragTexCoord;

out vec3 fragPos;
out vec3 fragNormal;

out vec3 oldPosition;
/*out vec3 newPosition;
out float waterDepth;
out float depth;*/

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform sampler2D tex;
//uniform sampler2D environment;

float eta = 0.7504;
int MAX_ITERATIONS = 100;

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

    float scale = 30;

    vec4 info = texture(tex, texCoord);
    float height = info.x;      
    vec3 normal =  vec3(info.z, sqrt(1.0 - dot(info.ba, info.ba)), info.w);

    trans *= buildTranslation(vec3(-scale/2, height-5+0.8, -scale/2));
    trans *= buildScaling(vec3(scale, 1, scale));

    oldPosition = vec3(trans*vec4(pos, 1.0));
    /*vec3 currentWorldPos = oldPosition;
    vec4 projectedWaterPosition = lightSpaceMatrix * trans * vec4(pos, 1.0);
    vec2 currentPosition = projectedWaterPosition.xy;
    vec2 coords = 0.5 + 0.5 * currentPosition;
    vec3 refracted = normalize(refract(vec3(0.01, -1.0, 0.0), normal, eta));
    vec4 projectedRefractionVector = lightSpaceMatrix * vec4(refracted, 1.0);
    vec3 refractedDirection = projectedRefractionVector.xyz;
    waterDepth = 0.5 + 0.5 * projectedWaterPosition.z / projectedWaterPosition.w;
    float currentDepth = projectedWaterPosition.z;
    vec4 env = texture(environment, 0.5 + 0.5 * (lightSpaceMatrix * vec4(currentWorldPos, 1.0)).xy);
    //float factor = 1/1024 / length(refractedDirection.xy);
    float factor = 1.0 / textureSize(environment, 0).x;
    vec2 deltaDirection = refractedDirection.xy * factor;
    float deltaDepth = refractedDirection.z * factor;
    for (int i = 0; i < MAX_ITERATIONS; i++) {
        // Move the coords in the direction of the refraction
        currentPosition += deltaDirection;
        currentDepth += deltaDepth;
        currentWorldPos += refracted * 0.002;
        float d = (lightSpaceMatrix * vec4(currentWorldPos, 1.0)).z;
        //newPosition += (vec4(refracted, 1.0)*inverse(lightSpaceMatrix)).xyz/1024;

        // End of loop condition: The ray has hit the environment
        if (env.w <= d) {
        break;
        }

        //env = texture(environment, 0.5 + 0.5 * currentPosition);
        env = texture(environment, 0.5 + 0.5 * (lightSpaceMatrix * vec4(currentWorldPos, 1.0)).xy);
    }
    //newPosition = env.xyz;
    newPosition = currentWorldPos;
    //newPosition = (inverse(lightSpaceMatrix)*vec4(currentPosition, currentDepth,1.0)).xyz;
    vec4 projectedEnvPosition = lightSpaceMatrix * vec4(newPosition, 1.0);
    depth = 0.5 + 0.5 * projectedEnvPosition.z / projectedEnvPosition.w;
    gl_Position = projectedEnvPosition;*/

	// FragPos = vec3(model * vec4(aPos, 1.0));
    fragPos = vec3(trans*vec4(pos, 1.0));
    fragNormal = normal;
    gl_Position = lightSpaceMatrix * trans * vec4(pos, 1.0);
    // fragPos = vec3(model*vec4(pos, 1.0));
    fragTexCoord = texCoord;
}