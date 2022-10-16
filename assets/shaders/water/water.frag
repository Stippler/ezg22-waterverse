#version 450 core

out vec4 fragColor;
	
in vec2 fragTexCoord;

uniform sampler2D tex;
	
void main()
{             
    // vec3 texCol = texture(tex, fragTexCoord).rgb;      
    // fragColor = vec4(texCol, 1.0);
    vec4 col = texture(tex, fragTexCoord);
    fragColor = vec4(6.0/255, 66.0/255, 115.0/255, 0.4);
    // fragColor = vec4(0, 0, 0, 1);
}