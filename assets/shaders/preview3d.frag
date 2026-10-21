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

// preview-only multiplier
uniform float uSpecBoost;

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

    vec3 N = (uUseNormalMap == 1)
        ? normalize(vTBN * (texture(uNormalMap, vUV).rgb * 2.0 - 1.0))
        : normalize(vTBN[2]);

    float roughness = (uUseRoughnessMap == 1)
        ? clamp(texture(uRoughnessMap, vUV).r, 0.0, 1.0)
        : 0.6;

    float ao = (uUseAOMap == 1) ? texture(uAOMap, vUV).r : 1.0;

    vec3 L = normalize(-uLightDir);
    vec3 V = normalize(uViewPos - vWorldPos);
    vec3 H = normalize(L + V);

    float ndl = max(dot(N, L), 0.0);
    float ndh = max(dot(N, H), 0.0);

    float ambient = clamp(uAmbient, 0.0, 1.0);
    float diffuse = ndl * max(uLightIntensity, 0.0);

    float r = max(roughness, 0.001);
    float shininessExp = mix(256.0, 4.0, r);
    float spec = pow(ndh, shininessExp);

    spec *= max(uSpecBoost, 0.0);

    // stronger by design for debugging visibility
    float specStrength = mix(1.2, 0.05, r);
    spec *= step(0.0, ndl) * ndl * specStrength * max(uLightIntensity, 0.0);

    vec3 color = albedo * ((ambient * ao + diffuse) * uLightColor) + vec3(spec);

    FragColor = vec4(color, 1.0);
}




//void main()
//{
//    float roughness = 0.6;
//    if (uUseRoughnessMap == 1)
//        roughness = clamp(texture(uRoughnessMap, vUV).r, 0.0, 1.0);
//
//    FragColor = vec4(vec3(roughness), 1.0);
//}

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
//    // Roughness (0 = shiny, 1 = rough)
//    float roughness = 0.6;
//    if (uUseRoughnessMap == 1)
//        roughness = clamp(texture(uRoughnessMap, vUV).r, 0.02, 1.0);
//
//    // AO
//    float ao = (uUseAOMap == 1) ? texture(uAOMap, vUV).r : 1.0;
//
//    vec3 L = normalize(-uLightDir);
//    vec3 V = normalize(uViewPos - vWorldPos);
//    vec3 H = normalize(L + V);
//
//    float ndl = max(dot(N, L), 0.0);
//    float ndh = max(dot(N, H), 0.0);
//
//    float ambient = clamp(uAmbient, 0.0, 1.0);
//    float diffuse = ndl * max(uLightIntensity, 0.0);
//
//    // IMPORTANT: don't re-clamp roughness to 0.25
//    float r = roughness;
//
//    // Specular response from roughness
//    float shininessExp = mix(128.0, 6.0, r);       // low roughness = tighter highlight
//    float spec = pow(ndh, shininessExp);
//
//    float specStrength = mix(0.30, 0.03, r);       // low roughness = stronger highlight
//    spec *= step(0.0, ndl) * ndl * specStrength * max(uLightIntensity, 0.0);
//
//    // Rim
//    float rim = 1.0 - max(dot(N, V), 0.0);
//    rim = pow(rim, max(uRimPower, 0.0001));
//    vec3 rimTerm = (uUseRim == 1) ? (uRimColor * rim * max(uRimIntensity, 0.0)) : vec3(0.0);
//
//    // Lighting: AO mostly affects ambient
//    vec3 baseLit = (ambient * ao + diffuse) * uLightColor;
//
//    // IMPORTANT: spec is not multiplied by albedo
//    vec3 color = albedo * baseLit + vec3(spec) * uLightColor + rimTerm;
//
//    FragColor = vec4(color, 1.0);
//}

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
//    float roughness = 0.6; // fallback
//    if (uUseRoughnessMap == 1)
//        roughness = clamp(texture(uRoughnessMap, vUV).r, 0.04, 1.0);
//
//
//    
//
//    vec3 L = normalize(-uLightDir);
//    vec3 V = normalize(uViewPos - vWorldPos);
//    vec3 H = normalize(L + V);
//
//    float ndl = max(dot(N, L), 0.0);
//    float ndh = max(dot(N, H), 0.0);
//
//    float ambient = clamp(uAmbient, 0.0, 1.0);
//    float diffuse = ndl * max(uLightIntensity, 0.0);
//
//    
//    // clamp roughness so it never gets mirror-smooth
//    float r = clamp(roughness, 0.25, 1.0);
//
//    float shininess = mix(64.0, 4.0, r);
//    float spec = pow(ndh, shininess);
//
//    // fade with light facing, and keep it modest
//    spec *= step(0.0, ndl) * ndl;
//    spec *= mix(0.35, 0.05, r) * max(uLightIntensity, 0.0);
//
//    
//
//
//    float rim = 1.0 - max(dot(N, V), 0.0);
//    rim = pow(rim, max(uRimPower, 0.0001));
//    vec3 rimTerm = (uUseRim == 1) ? (uRimColor * rim * max(uRimIntensity, 0.0)) : vec3(0.0);
//
//   float ao = (uUseAOMap == 1) ? texture(uAOMap, vUV).r : 1.0;
//    // use in the color line:
//    vec3 color = albedo * ((ambient * ao + diffuse) * uLightColor)
//           + albedo * spec * uLightColor + rimTerm;
//    vec3 lit = (ambient + diffuse) * uLightColor + rimTerm;
//    //vec3 color = albedo * lit + vec3(spec) * uLightColor;
//
//    
//    
//    FragColor = vec4(color, 1.0);
//}
//

