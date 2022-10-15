#version 450 core

out vec4 fragColor;
	
in vec2 fragTexCoord;

uniform sampler2D tex;
	
void main()
{             
    // vec3 texCol = texture(tex, fragTexCoord).rgb;      
    // fragColor = vec4(texCol, 1.0);
    vec4 col = texture(tex, fragTexCoord);
    fragColor = col;
    // fragColor = vec4(0, 0, 0, 1);
}