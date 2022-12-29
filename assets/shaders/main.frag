#version 450 core

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 1
uniform PointLight plight[NR_POINT_LIGHTS];
uniform mat4 lightSpaceMatrix;

float farPlane = 25.0;
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
float causticsBias = 1;
const vec2 resolution = vec2(1024.0);

// ssao
uniform sampler2D ssao;

// shadowMap
uniform sampler2D shadowMap;

// Water GBuffer
uniform sampler2D gWaterPosition;
uniform sampler2D gWaterNormal;
uniform sampler2D gWaterAlbedo;

uniform vec2 screenSize;

uniform mat4 view;
uniform mat4 projection;

// Blur for Caustics
float blur(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  float intensity = 0.0;
  vec2 off1 = vec2(1.3846153846) * direction;
  vec2 off2 = vec2(3.2307692308) * direction;
  intensity += texture(image, uv).r * 0.2270270270;
  intensity += texture(image, uv + (off1 / resolution)).r * 0.3162162162;
  intensity += texture(image, uv - (off1 / resolution)).r * 0.3162162162;
  intensity += texture(image, uv + (off2 / resolution)).r * 0.0702702703;
  intensity += texture(image, uv - (off2 / resolution)).r * 0.0702702703;
  return intensity;
}

float AmbientOcclusion = texture(ssao, vec2(gl_FragCoord.x-0.5, gl_FragCoord.y-0.5)/screenSize).r;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo,float shadow) {
    // float materialAmbient = 0.8;
    float materialShininess = 32;
    vec3 lightDir = normalize(-light.direction);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);

    // combine results
    vec3 res = vec3(0, 0, 0);

    vec3 ambient = light.ambient * albedo * AmbientOcclusion; // * AmbientOcclusion;
    vec3 diffuse = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * albedo;
    res += ambient;
    res += (1.0 - shadow) * diffuse;
    res += specular;
    return res; // (ambient + (1.0 - shadow) * diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float shadow) {
    float materialShininess = 32;
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
        light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * albedo * AmbientOcclusion;
    vec3 diffuse = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * albedo;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + (1.0-shadow)*(diffuse + specular));
}

// Shadow calculation
float shadowCalculation(vec4 fragPosLightSpace, vec3 norm) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).w;
    float currentDepth = projCoords.z;
    vec3 lightDir = normalize(-light.direction);
    float bias = max(0.05 * (1.0 - dot(norm, light.direction)), 0.005);
    //float bias = 0.05;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).w;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float shadowCalculationPointLight(vec3 fragPos, PointLight light)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - light.position;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(cubeShadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= farPlane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);  
    return shadow;
}  

void main() {
    vec4 modelPos = inverse(view)*texture(gPosition, texCoords);
    vec4 modelNormal = texture(gNormal, texCoords);
    vec4 modelAlbedo = texture(gAlbedo, texCoords);

    // wrong positions?
    vec4 waterPos = texture(gWaterPosition, texCoords);
    vec4 waterNormal = texture(gWaterNormal, texCoords);
    vec4 waterAlbedo = texture(gWaterAlbedo, texCoords);

    vec3 waterViewDir = normalize(viewPos - waterPos.xyz);
    vec3 waterLight = calcDirLight(light, waterNormal.xyz, waterViewDir, waterAlbedo.xyz, 0.5);

    vec4 caustic = texture(caustics, texCoords); // does not work?
    float ssao = texture(ssao, texCoords).r;

    vec3 model_light = vec3(0, 0, 0);
    vec3 viewDir = normalize(viewPos - modelPos.xyz);
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(modelPos.xyz,1.0);
    float shadow = shadowCalculation(fragPosLightSpace, modelNormal.xyz);
    model_light += calcDirLight(light, modelNormal.xyz, viewDir, modelAlbedo.xyz, shadow);
    
    for(int i = 0; i<NR_POINT_LIGHTS; i++){
        float shadowpl = shadowCalculationPointLight(modelPos.xyz, plight[i]);
        model_light += calcPointLight(plight[i], modelNormal.xyz, modelPos.xyz, viewDir, modelAlbedo.xyz, shadowpl);

        shadowpl = shadowCalculationPointLight(waterPos.xyz, plight[i]);
        waterLight += calcPointLight(plight[i], waterNormal.xyz, waterPos.xyz, viewDir, waterAlbedo.xyz, shadowpl);
    }

    
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float causticsDepth = texture(caustics, projCoords.xy).w;
    float closestDepth = texture(shadowMap, projCoords.xy).w;
    float currentDepth = projCoords.z;
    if (closestDepth > currentDepth - causticsBias) {
        // Percentage Close Filtering
        float causticsIntensityR = 0.5 * (
        blur(caustics, projCoords.xy + vec2(2,2)*(1.0 / textureSize(shadowMap, 0)), resolution, vec2(0., 0.5)) +
        blur(caustics, projCoords.xy + vec2(2,2)*(1.0 / textureSize(shadowMap, 0)), resolution, vec2(0.5, 0.))
        );
        float causticsIntensityG = 0.5 * (
        blur(caustics, projCoords.xy, resolution, vec2(0., 0.5)) +
        blur(caustics, projCoords.xy, resolution, vec2(0.5, 0.))
        );
        float causticsIntensityB = 0.5 * (
        blur(caustics, projCoords.xy + vec2(-2,-2)*(1.0 / textureSize(shadowMap, 0)), resolution, vec2(0., 0.5)) +
        blur(caustics, projCoords.xy + vec2(-2,-2)*(1.0 / textureSize(shadowMap, 0)), resolution, vec2(0.5, 0.))
        );

        //all_lights += vec3(causticsIntensityR, causticsIntensityG, causticsIntensityB);
        model_light *= vec3(causticsIntensityR, causticsIntensityG, causticsIntensityB);
    }

    fragColor = vec4(model_light + waterLight, 1); // (waterAlbedo + modelAlbedo) / 2;


    // vec4 dirShadow = texture(shadowMap, texCoords); HOW?
    // fragColor = vec4(ssao, ssao, ssao, 1);
    // fragColor = modelNormal;
}