#version 430 core
out vec4 FragColor;
  
//in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
    vec4 color = texture(texture_diffuse1, TexCoord);
    FragColor = color;
}