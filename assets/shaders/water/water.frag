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

out vec4 fragColor;
in vec2 fragTexCoord;

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
    vec3 norm = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPos);
    // float shadow = ShadowCalculation(FragPosLightSpace, norm);
    float shadow = 0.0;
    vec3 allLights = vec3(0, 0, 0);
    allLights += calcDirLight(light, norm, viewDir, shadow);
    // for(int i = 0; i<NR_POINT_LIGHTS; i++){
    //     float shadowpl = ShadowCalculationPointLight(FragPos, plight[i]);
    //     all_lights += CalcPointLight(plight[i], norm, FragPos, viewDir, shadowpl);
    // }
    // vec3 texCol = texture(tex, fragTexCoord).rgb;      
    // fragColor = vec4(texCol, 1.0);
    // vec4 col = texture(tex, fragTexCoord);
    // fragColor = vec4(6.0 / 255, 66.0 / 255, 115.0 / 255, 0.8);
    // fragColor = vec4(6.0 / 255, 66.0 / 255, 115.0 / 255, 1);
    vec3 color = vec3(6.0 / 255, 66.0 / 255, 115.0 / 255);
    color = vec3(.4,.8,1);
    fragColor = vec4(allLights*color, 0.9);
    // fragColor = vec4(0, 0, 0, 1);
}