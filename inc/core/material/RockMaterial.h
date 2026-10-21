#pragma once
#include <core/Helper.h>   // Maps, Image, Hash, Smooth, Put, Clamp01

struct RockSettings
{
    int   size = 1024;

    float scale = 4.0f;        // base feature frequency
    float detail = 0.5f;       // 0..1 fine-noise mix
    float depth = 0.6f;        // height contrast
    float cracks = 0.35f;      // crack strength 0..1
    float variation = 0.20f;
    float shininess = 0.0f;    // 0 dry -> 1 wet

    float rockColorA[3] = { 0.55f, 0.53f, 0.50f }; // light
    float rockColorB[3] = { 0.25f, 0.24f, 0.23f }; // dark
};

Maps GenerateRockMaps(const RockSettings& s);