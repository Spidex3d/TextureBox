#version 460 core
in vec2 vUV;
in vec3 vWorldPos;
in mat3 vTBN;

out vec4 FragColor;

uniform sampler2D uAlbedo;
uniform sampler2D uNormalMap;
uniform int uUseNormalMap;

// NEW
uniform sampler2D uRoughnessMap;
uniform int uUseRoughnessMap;
// new
uniform sampler2D uAOMap;
uniform int uUseAOMap;

uniform vec3 uLightDir;
uniform float uLightIntensity;
uniform float uAmbient;
uniform vec3 uLightColor;

uniform vec3 uViewPos;
uniform int uUseRim;
uniform vec3 uRimColor;
uniform float uRimPower;
uniform float uRimIntensity;

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

    float roughness = 0.6; // fallback
    if (uUseRoughnessMap == 1)
        roughness = clamp(texture(uRoughnessMap, vUV).r, 0.04, 1.0);


    

    vec3 L = normalize(-uLightDir);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 H = normalize(L + V);

    float ndl = max(dot(N, L), 0.0);
    float ndh = max(dot(N, H), 0.0);

    float ambient = clamp(uAmbient, 0.0, 1.0);
    float diffuse = ndl * max(uLightIntensity, 0.0);

    
    // clamp roughness so it never gets mirror-smooth
    float r = clamp(roughness, 0.25, 1.0);

    float shininess = mix(64.0, 4.0, r);
    float spec = pow(ndh, shininess);

    // fade with light facing, and keep it modest
    spec *= step(0.0, ndl) * ndl;
    spec *= mix(0.35, 0.05, r) * max(uLightIntensity, 0.0);

    


    float rim = 1.0 - max(dot(N, V), 0.0);
    rim = pow(rim, max(uRimPower, 0.0001));
    vec3 rimTerm = (uUseRim == 1) ? (uRimColor * rim * max(uRimIntensity, 0.0)) : vec3(0.0);

   float ao = (uUseAOMap == 1) ? texture(uAOMap, vUV).r : 1.0;
    // use in the color line:
    vec3 color = albedo * ((ambient * ao + diffuse) * uLightColor)
           + albedo * spec * uLightColor + rimTerm;
    vec3 lit = (ambient + diffuse) * uLightColor + rimTerm;
    //vec3 color = albedo * lit + vec3(spec) * uLightColor;

    
    
    FragColor = vec4(color, 1.0);
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
//
//uniform vec3 uLightDir;       // world-space direction (points FROM light)
//uniform float uLightIntensity;
//uniform float uAmbient;
//uniform vec3 uLightColor;
//// rim light
//uniform vec3 uViewPos;
//uniform int uUseRim;
//uniform vec3 uRimColor;
//uniform float uRimPower;
//uniform float uRimIntensity;
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
//        N = normalize(vTBN[2]);
//    }
//
//    vec3 L = normalize(-uLightDir);
//    float ndl = max(dot(N, L), 0.0);
//
//    float ambient = clamp(uAmbient, 0.0, 1.0);
//    float diffuse = ndl * max(uLightIntensity, 0.0);
//
//    vec3 V = normalize(uViewPos - vWorldPos);
//    float rim = 1.0 - max(dot(N, V), 0.0);
//    rim = pow(rim, max(uRimPower, 0.0001));
//    vec3 rimTerm = (uUseRim == 1) ? (uRimColor * rim * max(uRimIntensity, 0.0)) : vec3(0.0);
//
//    vec3 lit = (ambient + diffuse) * uLightColor + rimTerm;
//    vec3 color = albedo * lit;
//    FragColor = vec4(color, 1.0);
//
//}


