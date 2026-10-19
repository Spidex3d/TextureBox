#pragma once
#include <core/Image.h>


struct RoughnessMapSettings
{
    float strength = 2.0f;   // bigger 
    bool invertY = false;    // toggle 
    bool wrap = false;       // tile
};
// Returns an RGBA image (same size as source)
Image GenerateRoughnessMap(const Image& source, const RoughnessMapSettings& settings);
