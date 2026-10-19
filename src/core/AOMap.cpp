#include "core/AOMap.h"
#include <algorithm>
#include <cstdint>

Image GenerateAOMap(const Image& source, const AOSettings& settings)
{
    Image out;
    if (!source.IsValid()) return out;

    out.width = source.width;
    out.height = source.height;
    out.pixels.resize((size_t)source.width * source.height * 4);

    const float k = std::max(settings.strength, 0.01f);

    for (int i = 0; i < source.width * source.height; ++i)
    {
        const uint8_t* p = &source.pixels[i * 4];
        float g = (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) / 255.0f;

        // darker creases feel more "occluded"
        float ao = 1.0f - g;
        ao = (ao - 0.5f) * k + 0.5f;
        ao = std::clamp(ao, 0.0f, 1.0f);

        if (settings.invert) ao = 1.0f - ao;

        const uint8_t v = (uint8_t)(ao * 255.0f);
        uint8_t* o = &out.pixels[i * 4];
        o[0] = o[1] = o[2] = v;
        o[3] = 255;
    }

    return out;
}