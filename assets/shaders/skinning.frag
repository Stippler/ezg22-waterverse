#version 450 core
out vec4 FragColor;

//in vec3 ourColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;
in vec4 FragPosLightSpace;
// flat in ivec4 Bones;
// in vec4 W;
#define NR_POINT_LIGHTS 1  

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D shadowMap;
uniform sampler2D ssao;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform samplerCube cubeShadowMap;
uniform sampler2D caustics;
float causticsBias = 0.001;
const vec2 resolution = vec2(1024.0);

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

uniform DirLight light;

//uniform PointLight plight;
uniform PointLight plight[NR_POINT_LIGHTS];
float far_plane = 25.0;

uniform vec3 viewPos;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

uniform vec2 screenSize;

float AmbientOcclusion = texture(ssao, vec2(gl_FragCoord.x-0.5, gl_FragCoord.y-0.5)/screenSize).r;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 res = vec3(0, 0, 0);
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoord)) * AmbientOcclusion;
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoord));
    res += ambient;
    res += (1.0-shadow)*diffuse;
    res += specular;
    return res; // (ambient + (1.0 - shadow) * diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
        light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoord)) * AmbientOcclusion;
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoord));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + (1.0-shadow)*(diffuse + specular));
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 norm) {
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

float ShadowCalculationPointLight(vec3 fragPos, PointLight light)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - light.position;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(cubeShadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);  
    return shadow;
}  

void main() {
    vec3 ambient = light.ambient * material.ambient;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    float shadow = ShadowCalculation(FragPosLightSpace, norm);
    vec3 all_lights = vec3(0, 0, 0);
    all_lights += CalcDirLight(light, norm, viewDir, shadow);
    for(int i = 0; i<NR_POINT_LIGHTS; i++){
        float shadowpl = ShadowCalculationPointLight(FragPos, plight[i]);
        all_lights += CalcPointLight(plight[i], norm, FragPos, viewDir, shadowpl);
    }
    // all_lights += CalcPointLight(plight, norm, FragPos, viewDir);

    //Debug vertex skinning
    /*bool found = false;
	int bone = 4; 
	int k = 0;
	for(int i = 0; i < 4; i++){
		if(Bones[i] == bone){
			//k = i;
			if(W[i] > 0.5){
				vec3 color = vec3(0.0, 1.0, 0.0);
				FragColor = vec4(color, 1.0);
				found = true;
			}
			else if(W[i] > 0){
				vec3 color = vec3(1.0, 0.0, 0.0);
				FragColor = vec4(color, 1.0);
				found = true;
			}
			else if(W[i] == 0){
				vec3 color = vec3(0.0, 0.0, 1.0);
				FragColor = vec4(color, 1.0);
				found = true;
			}
			break;
		}
	}
	if(!found){
		FragColor = vec4(all_lights, 1.0);
	}*/
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float causticsDepth = texture(caustics, projCoords.xy).w;
    float closestDepth = texture(shadowMap, projCoords.xy).w;
    float currentDepth = projCoords.z;
    if (closestDepth > currentDepth - 0.05) {
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
        all_lights *= vec3(causticsIntensityR, causticsIntensityG, causticsIntensityB);
    }
    
    FragColor = vec4(vec3(0.4, 0.9, 1)*all_lights, 1.0);
    // vec2 uv = vec2((gl_FragCoord.x-0.5)/800, (gl_FragCoord.y-0.5)/600);
    // vec3 ledl = normalize(texture(gNormal, uv).rgb);
    // FragColor = vec4(ledl, 1.0);
    // FragColor = vec4(AmbientOcclusion, AmbientOcclusion, AmbientOcclusion, 1.0);
}