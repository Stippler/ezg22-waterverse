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
uniform mat4 lightSpaceMatrix;
uniform sampler2D tex;
uniform sampler2D environment;

layout (location = 0) out vec4 caustics;
in vec2 fragTexCoord;

float causticsFactor = 4;

in vec3 oldPosition;
/*in vec3 newPosition;
in float waterDepth;
in float depth;*/

float eta = 0.7504;
int MAX_ITERATIONS = 100;

void main() {

    vec3 currentWorldPos = oldPosition;
    vec4 projectedWaterPosition = lightSpaceMatrix * vec4(oldPosition, 1.0);
    vec2 currentPosition = projectedWaterPosition.xy;
    vec2 coords = 0.5 + 0.5 * currentPosition;
    vec3 refracted = refract(vec3(0.01, -1.0, 0.0), fragNormal, eta);
    vec4 projectedRefractionVector = lightSpaceMatrix * vec4(refracted, 1.0);
    vec3 refractedDirection = projectedRefractionVector.xyz;
    float waterDepth = 0.5 + 0.5 * projectedWaterPosition.z / projectedWaterPosition.w;
    float currentDepth = projectedWaterPosition.z;
    //vec4 env = texture(environment, 0.5 + 0.5 * (lightSpaceMatrix * vec4(currentWorldPos, 1.0)).xy);
    vec4 env = texture(environment, coords);
    float factor = 1/1024 / length(refractedDirection.xy);
    //float factor = 1.0 / textureSize(environment, 0).x;
    vec2 deltaDirection = refractedDirection.xy * factor;
    float deltaDepth = refractedDirection.z * factor;
    for (int i = 0; i < MAX_ITERATIONS; i++) {
        // Move the coords in the direction of the refraction
        currentPosition += deltaDirection;
        currentDepth += deltaDepth;
        currentWorldPos += refracted * 0.001;
        float d = (lightSpaceMatrix * vec4(currentWorldPos, 1.0)).z;
        //newPosition += (vec4(refracted, 1.0)*inverse(lightSpaceMatrix)).xyz/1024;

        // End of loop condition: The ray has hit the environment
        if (env.w <= d) {
        break;
        }

        //env = texture(environment, 0.5 + 0.5 * currentPosition);
        env = texture(environment, 0.5 + 0.5 * (lightSpaceMatrix * vec4(currentWorldPos, 1.0)).xy);
    }
    //vec3 newPosition = env.xyz;
    vec3 newPosition = currentWorldPos;
    //newPosition = (inverse(lightSpaceMatrix)*vec4(currentPosition, currentDepth,1.0)).xyz;
    vec4 projectedEnvPosition = lightSpaceMatrix * vec4(newPosition, 1.0);
    float depth = 0.5 + 0.5 * projectedEnvPosition.z / projectedEnvPosition.w;

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
    
    caustics = vec4(causticsIntensity, 0, 0, depth);
}