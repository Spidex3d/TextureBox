#pragma once
#include <core/Image.h>


struct DisplacementMapSettings
{
    float strength = 2.0f;   // bigger 
    bool invertY = false;    // toggle 
    bool wrap = false;       // tile
};

// Returns an RGBA image (same size as source)
Image GenerateDisplacementMap(const Image& source, const DisplacementMapSettings& settings);
