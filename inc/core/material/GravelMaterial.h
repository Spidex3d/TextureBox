#pragma once
#include <core/Helper.h>   // Maps, Image, Hash, Smooth, Put, Clamp01

struct GravelSettings
{
    int   size = 1024;

    float scale = 2.0f;             // base feature frequency
    float detail = 0.5f;            // 0..1 fine-noise mix
    float depth = 0.6f;             // height contrast
	int   stoneCount = 100;         // starting number of gravel pieces
	float variation = 0.20f;        // per-gravel size variation
	float colorVariation = 0.20f;   // per-gravel colour variation
	float pitting = 0.08f;          // the face could be pitted, 0..1
	float veins = 0.0f;             // 0..1 vein strength white lines in the face of the gravel
    float shininess = 0.0f;         // 0 dry -> 1 wet
    float irregularity = 0.6f;  // 0 round -> 1 very angular/lumpy/elongated
    float coverage = 1.0f;  // 0 = bare cobble pavement, 1 = full gravel on top

    float gravelColorA[3] = { 0.55f, 0.53f, 0.50f }; // light
    float gravelColorB[3] = { 0.25f, 0.24f, 0.23f }; // dark
};

Maps GenerateGravelMaps(const GravelSettings & s);
