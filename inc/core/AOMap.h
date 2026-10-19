#pragma once
#include <core/Image.h>

struct AOSettings
{
    float strength = 1.5f; // contrast/intensity
    bool invert = false;
};

Image GenerateAOMap(const Image& source, const AOSettings& settings);