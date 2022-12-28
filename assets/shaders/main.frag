#version 450 core

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 fragColor;

in vec2 texCoords;

// Lights
uniform DirLight light;
uniform vec3 viewPos;

// Model Textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gEnvironment;

// Shadow Textures
uniform samplerCube cubeShadowMap;

// Caustics
uniform sampler2D caustics;

// ssao
uniform sampler2D ssao;

// shadowMap
uniform sampler2D shadowMap;

// Water GBuffer
uniform sampler2D gWaterPosition;
uniform sampler2D gWaterNormal;
uniform sampler2D gWaterAlbedo;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow) {
    // float materialAmbient = 0.8;
    float materialShininess = 3;
    vec3 lightDir = normalize(-vec3(0.0, -1.0, 0));

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);

    // combine results
    vec3 res = vec3(0, 0, 0);

    vec3 ambient = light.ambient; // * AmbientOcclusion;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    res += ambient;
    res += (1.0 - shadow) * diffuse;
    res += specular;
    return res; // (ambient + (1.0 - shadow) * diffuse + specular);
}

void main() {
    vec4 modelPos = texture(gPosition, texCoords);
    vec4 modelNormal = texture(gNormal, texCoords);
    vec4 modelAlbedo = texture(gAlbedo, texCoords);

    // wrong positions?
    vec4 waterPos = texture(gWaterPosition, texCoords);
    vec4 waterNormal = texture(gWaterNormal, texCoords);
    vec4 waterAlbedo = texture(gWaterAlbedo, texCoords);

    // vec3 viewDir = normalize(viewPos-waterPos.xyz)
    // vec3 waterLight = calcDirLight(light, waterNormal.xyz, )

    vec4 caustic = texture(caustics, texCoords); // does not work?
    float ssao = texture(ssao, texCoords).r;
    // vec4 dirShadow = texture(shadowMap, texCoords); HOW?
    // fragColor = vec4(ssao, ssao, ssao, 1);
    fragColor = (waterAlbedo+modelAlbedo)/2;
}