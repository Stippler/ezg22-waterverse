#version 450 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

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
uniform PointLight plight;

uniform vec3 viewPos;
float AmbientOcclusion = texture(ssao, TexCoord).r;
vec3 Diffuse = texture(gAlbedo, TexCoord).rgb;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, float shadow) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 res = vec3(0, 0, 0);
    vec3 ambient = light.ambient * Diffuse * AmbientOcclusion;
    vec3 diffuse = light.diffuse * diff * Diffuse;
    vec3 specular = light.specular * spec * Diffuse;
    res += ambient;
    res += (1.0-shadow)*diffuse;
    res += specular;
    return res; // (ambient + (1.0 - shadow) * diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
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
    vec3 ambient = light.ambient * Diffuse * AmbientOcclusion;
    vec3 diffuse = light.diffuse * diff * Diffuse;
    vec3 specular = light.specular * spec * Diffuse;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoord).rgb;
    //vec3 Normal = texture(gNormal, TexCoord).rgb;
    vec3 norm = normalize(texture(gNormal, TexCoord).rgb);
    
    // then calculate lighting as usual
    vec3 ambient = vec3(0.3 * Diffuse * AmbientOcclusion);
    //vec3 lighting  = ambient; 
    //vec3 viewDir  = normalize(-FragPos); // viewpos is (0.0.0)
    vec3 viewDir = normalize(viewPos - FragPos);
    // diffuse
    /*
    vec3 lightDir = normalize(light.position - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * light.color;
    // speculard
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = light.color * spec;
    // attenuation
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * distance * distance);
    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;
    */

    //float shadow = ShadowCalculation(FragPosLightSpace, norm);
    vec3 lighting = vec3(0, 0, 0);
    lighting += CalcDirLight(light, norm, viewDir, 1.0f);

    //FragColor = vec4(lighting, 1.0);
    FragColor = vec4(norm, 1.0);
    // FragColor = vec4(norm.x, norm.y, norm.z, 1.0);
}