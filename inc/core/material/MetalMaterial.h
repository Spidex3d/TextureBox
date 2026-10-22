#pragma once
#include <core/Helper.h>   // Maps, Image, Hash, Smooth, Put, Clamp01

struct MetalSettings
{
    int   size = 1024;

    float scale = 4.0f;         // base feature frequency
    float detail = 0.05f;       // 0..1 fine-noise mix
    float depth = 0.01f;        // height contrast
    float rust = 0.35f;         // rust strength 0..1
	float moss = 0.35f;         // moss strength 0..1
    float scratches = 0.35f;     // scratch strength 0..1
	float variation = 0.01f;    // per-pixel colour variation
	float shininess = 0.0f;     // 0 dull -> 1 shiny

    float metalColorA[3] = { 0.55f, 0.53f, 0.50f };     // light
    float metalColorB [3] = { 0.25f, 0.24f, 0.23f };    // dark
	float rustColor[3] = { 0.45f, 0.20f, 0.07f };       // rust
	float mossColor[3] = { 0.30f, 0.50f, 0.20f };       // moss
};

Maps GenerateMetalMaps(const MetalSettings& s);
