#version 430 core
out vec4 FragColor;
  
//in vec3 ourColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}; 

uniform Light light;

uniform vec3 viewPos;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 
  
uniform Material material;

void main()
{
    vec3 ambient = light.ambient * material.ambient;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (diff * material.diffuse) * light.diffuse;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = (material.specular * spec) * light.specular;

    vec3 light = (ambient + diffuse + specular);
    vec3 texture = texture(texture_diffuse1, TexCoord).rgb;
    FragColor = vec4(light*texture, 1.0);
}