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

uniform sampler2D background;

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

float ambientOcclusion = texture(ssao, vec2(gl_FragCoord.x - 0.5, gl_FragCoord.y - 0.5) / screenSize).r;

vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float shadow) {
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

    vec3 ambient = light.ambient * albedo * ambientOcclusion; // * AmbientOcclusion;
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
    vec3 ambient = light.ambient * albedo * ambientOcclusion;
    vec3 diffuse = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * albedo;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + (1.0 - shadow) * (diffuse + specular));
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
vec3 gridSamplingDisk[20] = vec3[](vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1), vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0), vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1), vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1));

float shadowCalculationPointLight(vec3 fragPos, PointLight light) {
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - light.position;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0;
    for(int i = 0; i < samples; ++i) {
        float closestDepth = texture(cubeShadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= farPlane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
    return shadow;
}

float inCube(vec3 pos) {
    float size = 14.5;
    vec3 bottomLeft = vec3(-size);
    vec3 topRight = vec3(size);
    vec3 s = step(bottomLeft, pos) - step(topRight, pos);
    return s.x * s.y * s.z;
}

vec3 rayMarching(vec3 viewPos, vec3 modelPos) {
    vec3 dir = modelPos - viewPos;
    float size = length(dir);
    if(abs(size) < 0.00001) {
        size = farPlane;
    } else {
        dir /= size;
    }
    return vec3(size, size, size);
}


// This assumes the pixel position px to be in [0,1],
// which can be done by (x+0.5)/w or (y+0.5)/h (or h-y +0.5 for screens
// with top left origin) to sample pixel centers
vec3 createRay(vec2 px, mat4 PInv, mat4 VInv)
{
  
    // convert pixel to NDS
    // [0,1] -> [-1,1]
    vec2 pxNDS = px*2. - 1.;

    // choose an arbitrary point in the viewing volume
    // z = -1 equals a point on the near plane, i.e. the screen
    vec3 pointNDS = vec3(pxNDS, -1.);

    // as this is in homogenous space, add the last homogenous coordinate
    vec4 pointNDSH = vec4(pointNDS, 1.0);
    // transform by inverse projection to get the point in view space
    vec4 dirEye = PInv * pointNDSH;

    // since the camera is at the origin in view space by definition,
    // the current point is already the correct direction 
    // (dir(0,P) = P - 0 = P as a direction, an infinite point,
    // the homogenous component becomes 0 the scaling done by the 
    // w-division is not of interest, as the direction in xyz will 
    // stay the same and we can just normalize it later
    dirEye.w = 0.;

    // compute world ray direction by multiplying the inverse view matrix
    vec3 dirWorld = (VInv * dirEye).xyz;

    // now normalize direction
    return normalize(dirWorld); 
}

void main() {
    // TODO: calculate once outside of shader:
    mat4 vInv = inverse(view);
    mat4 pInv = inverse(projection);
    // vec2 px = texCoords*2-vec2(1, 1);
    vec3 ray = createRay(texCoords, pInv, vInv);

    vec4 modelPos = vInv * texture(gPosition, texCoords);
    vec4 modelNormal = texture(gNormal, texCoords);
    vec4 modelAlbedo = texture(gAlbedo, texCoords);

    vec4 backgroundAlbedo = texture(background, texCoords);

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
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(modelPos.xyz, 1.0);
    float shadow = shadowCalculation(fragPosLightSpace, modelNormal.xyz);
    model_light += calcDirLight(light, modelNormal.xyz, viewDir, modelAlbedo.xyz, shadow);

    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
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
    if(closestDepth > currentDepth - causticsBias) {
        // Percentage Close Filtering
        float causticsIntensityR = 0.5 * (blur(caustics, projCoords.xy + vec2(2, 2) * (1.0 / textureSize(shadowMap, 0)), resolution, vec2(0., 0.5)) +
            blur(caustics, projCoords.xy + vec2(2, 2) * (1.0 / textureSize(shadowMap, 0)), resolution, vec2(0.5, 0.)));
        float causticsIntensityG = 0.5 * (blur(caustics, projCoords.xy, resolution, vec2(0., 0.5)) +
            blur(caustics, projCoords.xy, resolution, vec2(0.5, 0.)));
        float causticsIntensityB = 0.5 * (blur(caustics, projCoords.xy + vec2(-2, -2) * (1.0 / textureSize(shadowMap, 0)), resolution, vec2(0., 0.5)) +
            blur(caustics, projCoords.xy + vec2(-2, -2) * (1.0 / textureSize(shadowMap, 0)), resolution, vec2(0.5, 0.)));

        model_light *= vec3(causticsIntensityR, causticsIntensityG, causticsIntensityB);
    }

    float maxRaySize = 70;
    float rayLength;
    float hitModel=0.0;
    if(modelNormal == vec4(0, 0, 0, 1)) {
        rayLength = maxRaySize;
    } else {
        rayLength = length(modelPos.xyz - viewPos);
        hitModel=1.0;
    }

    float hitCube = 1;
    if(waterNormal == vec4(0,0,0,1)){
        hitCube = 0;
    }

    vec3 curPos = viewPos;
    int totalSamples = 500;
    float numSamples = totalSamples*(rayLength/maxRaySize);
    float cubeCount = 0;
    float shadowCount = 0;
    float lightCount = 0;
    float stepSize = maxRaySize/totalSamples;
    float average_light_depth=0.0;
    for(int i = 0; i < numSamples; i++) {
        curPos = viewPos+i*ray*stepSize;
        float in_cube = inCube(curPos);
        cubeCount += in_cube;
        // inCube -> 0 outside
        //        -> 1 otherwise
        // calc average distance to light source?
        average_light_depth += in_cube*(curPos.y-15)/30*-1;

        // vec4 pos_lightspace = lightSpaceMatrix*vec4(curPos, 1.0);
        // vec4 pos_lightspace = lightSpaceMatrix*vec4(curPos, 1.0);
        // float dist_lightspace = pos_lightspace.y;
        // average_light_depth += dist_lightspace*in_cube;
        // TODO: volumetric directional light:
        // vec3 currProj = curPosLightSpace.xyz / curPosLightSpace.w;
        // currProj = currProj * 0.5 + 0.5;
        // float closestDepth = texture(shadowMap, currProj.xy).w;
        // float currDepth = currProj.z;
        // if(currDepth-0.01 > closestDepth) {
        //     //we are in shadow
        //     shadowCount+=1.0;
        //     lightCount+=1.0;
        // }
    }

    average_light_depth /= max(cubeCount, 1);
    float shadowFrac = shadowCount/1000;
    float lightFrac = lightCount/numSamples;

    float cubeFrac = cubeCount/numSamples;
    vec3 cubeColor = vec3(average_light_depth, average_light_depth/2, average_light_depth/3)/4;

    float inside = inCube(viewPos);
    float water_t = (1-inside)*0.4;
    float model_t = 1-water_t;

    vec3 light = (1-hitModel)*(vec3(backgroundAlbedo)-(1-inside)*hitCube*vec3(0.17))+
                 (model_t*model_light+(water_t*waterAlbedo.xyz))-cubeColor;
    fragColor = vec4(light, 1);
    // vec4(light+vec3(0.3, 0.3, 0.3), 1);
    // fragColor = vec3(waterLight, 1);
    // fragColor = (waterAlbedo+backgroundAlbedo)/2;
    // fragColor = waterAlbedo;
    // fragColor = backgroundAlbedo;
    // fragColor = vec4(cubeColor, 1);
    // fragColor = vec4(ssao, ssao, ssao, 1);
}