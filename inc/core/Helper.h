#pragma once
#include <core/Image.h>

#include <algorithm>
#include <cmath>
#include <cstdint>

static inline float Clamp01(float v) { return std::clamp(v, 0.0f, 1.0f); }


struct Maps
{
    Image albedo;
    Image height;
    Image roughness;
    Image ao;
};

static float Hash(int x, int y)
{
    unsigned int h = (unsigned int)(x * 374761393 + y * 668265263);
    h = (h ^ (h >> 13)) * 1274126177u;
    return ((h ^ (h >> 16)) & 0xFFFFFF) / float(0xFFFFFF);
}

static float Smooth(float a, float b, float x)
{
    float t = std::clamp((x - a) / (b - a), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

static void Put(Image& img, int i, float r, float g, float b)
{
    uint8_t* p = &img.pixels[i * 4];
    p[0] = (uint8_t)(std::clamp(r, 0.0f, 1.0f) * 255.0f);
    p[1] = (uint8_t)(std::clamp(g, 0.0f, 1.0f) * 255.0f);
    p[2] = (uint8_t)(std::clamp(b, 0.0f, 1.0f) * 255.0f);
    p[3] = 255;
}
