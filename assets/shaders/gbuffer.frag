#version 450 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = vec4(FragPos, 1);
    // also store the per-fragment normals into the gbuffer
    gNormal = vec4(normalize(Normal), 1);
    // gNormal = normalize(vec3(0.0,0.0,0.0));
    // and the diffuse per-fragment color
    gAlbedo = vec4(vec3(texture(texture_diffuse1, TexCoord).rgb), 1);

	gl_FragDepth = gl_FragCoord.z;
}