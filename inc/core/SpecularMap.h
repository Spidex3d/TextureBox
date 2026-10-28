#pragma once
#include <core/Image.h>


struct SpecularMapSettings
{
    float strength = 2.0f;   // bigger 
    bool invertY = false;    // toggle 
    bool wrap = false;       // tile
};

// Returns an RGBA image (same size as source)
Image GenerateSpecularMap(const Image& source, const SpecularMapSettings& settings);
