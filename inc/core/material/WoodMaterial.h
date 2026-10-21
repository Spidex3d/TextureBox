#pragma once
#include <core/image.h>
#include <core/Helper.h>

struct WoodSettings
{
    int   size = 1024;

    // wood layout
    float rings = 9.0f;          // ring frequency
    float grainScale = 18.0f;    // fine grain frequency
    float warp = 0.12f;          // ring distortion amount


    // look
    float variation = 0.20f;     // per-plank/random variation
    float shininess = 0.25f;     // 0 matte -> 1 varnished
    float pitting = 0.03f;       // subtle surface breakup

    // plank lines (optional)
    float plankCount = 6.0f;     // across U
    float seamWidth = 0.015f;    // seam thickness

    float woodColorA[3] = { 0.52f, 0.33f, 0.17f }; // light tone
    float woodColorB[3] = { 0.35f, 0.21f, 0.10f }; // dark tone
};

Maps GenerateWoodMaps(const WoodSettings& s);
