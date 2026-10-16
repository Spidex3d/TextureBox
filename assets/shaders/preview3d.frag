#version 460 core
in vec2 vUV;
in vec3 vWorldPos;
in mat3 vTBN;

out vec4 FragColor;

uniform sampler2D uAlbedo;
uniform sampler2D uNormalMap;
uniform int uUseNormalMap;

uniform vec3 uLightDir;       // world-space direction (points FROM light)
uniform float uLightIntensity;
uniform float uAmbient;
uniform vec3 uLightColor;

void main()
{
    vec3 albedo = texture(uAlbedo, vUV).rgb;

    vec3 N;
    if (uUseNormalMap == 1)
    {
        vec3 nTex = texture(uNormalMap, vUV).rgb * 2.0 - 1.0;
        N = normalize(vTBN * nTex);
    }
    else
    {
        N = normalize(vTBN[2]);
    }

    vec3 L = normalize(-uLightDir);
    float ndl = max(dot(N, L), 0.0);

    float ambient = clamp(uAmbient, 0.0, 1.0);
    float diffuse = ndl * max(uLightIntensity, 0.0);

    vec3 lit = (ambient + diffuse) * uLightColor;
    vec3 color = albedo * lit;
    FragColor = vec4(color, 1.0);

//    float ambient = clamp(uAmbient, 0.0, 1.0);
//    float diffuse = ndl * max(uLightIntensity, 0.0);
//
//    vec3 color = albedo * (ambient + diffuse);
//    FragColor = vec4(color, 1.0);
}


//#version 460 core
//in vec2 vUV;
//in vec3 vWorldPos;
//in mat3 vTBN;
//
//out vec4 FragColor;
//
//uniform sampler2D uAlbedo;
//uniform sampler2D uNormalMap;
//uniform int uUseNormalMap;
//uniform vec3 uLightDir;
//
//void main()
//{
//    vec3 albedo = texture(uAlbedo, vUV).rgb;
//
//    vec3 N;
//    if (uUseNormalMap == 1)
//    {
//        vec3 nTex = texture(uNormalMap, vUV).rgb * 2.0 - 1.0;
//        N = normalize(vTBN * nTex);
//    }
//    else
//    {
//        N = normalize(vTBN[2]); // vertex normal in world space
//    }
//
//    vec3 L = normalize(-uLightDir);
//    float ndl = max(dot(N, L), 0.0);
//
//    vec3 color = albedo * (0.15 + 0.85 * ndl);
//    FragColor = vec4(color, 1.0);
//}
//