#pragma once
#include <core/Helper.h>   // Maps, Image, Hash, Smooth, Put, Clamp01

struct DirtSettings
{
    int   size = 1024;

    float scale = 2.0f;             // base feature frequency
    float detail = 0.5f;            // 0..1 fine-noise mix
    float depth = 0.6f;             // height contrast
    float variation = 0.20f;        // per-dirt size variation
	int dirtAmount = 100;           // starting number of dirt pieces
    float clodSize = 0.20f;         // size of each dirt clod
    float colorVariation = 0.20f;   // per-dirt colour variation
    float shininess = 0.0f;         // 0 dry -> 1 wet

    float dirtColorA[3] = { 0.55f, 0.53f, 0.50f }; // light
    float dirtColorB[3] = { 0.25f, 0.24f, 0.23f }; // dark
};

Maps GenerateDirtMaps(const DirtSettings& s);

