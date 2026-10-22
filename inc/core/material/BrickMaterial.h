#pragma once
#include <core/image.h>
#include <core\Helper.h>


struct BrickSettings
{
    int   cols = 6;
    int   rows = 12;
    float mortar = 0.08f;   // mortar width, fraction of a brick
    float bevel = 0.05f;    // soft edge width
    float variation = 0.25f; // per-brick colour variation
    int   size = 1024;

    float brickColor[3] = { 0.60f, 0.22f, 0.15f };
    float mortarColor[3] = { 0.55f, 0.53f, 0.50f };
    
    float edgeSoftMul = 1.6f;
    float pitting = 0.08f;
    float shininess = 0.0f; // 0 = matt brick, 1 = glazed tile
};


Maps GenerateBrickMaps(const BrickSettings& s);