#pragma once
#include <core/Image.h>

struct NormalMapSettings
{
    float strength = 2.0f;   // bigger = stronger bumps
    bool invertY = false;    // toggle for different normal conventions
    bool wrap = false;       // tile-friendly edges
};

// Returns an RGBA image (same size as source)
Image GenerateNormalMap(const Image& source, const NormalMapSettings& settings);