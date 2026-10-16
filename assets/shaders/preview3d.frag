#version 460 core
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uAlbedo;
uniform vec3 uLightDir;

void main()
{
    vec3 N = normalize(vNormal);
    float ndl = max(dot(N, normalize(-uLightDir)), 0.0);
    vec3 albedo = texture(uAlbedo, vUV).rgb;
    vec3 color = albedo * (0.15 + 0.85 * ndl);
    FragColor = vec4(color, 1.0);
}