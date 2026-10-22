#include "core/DisplacementMap.h"
#include <algorithm>
#include <cstdint>

Image GenerateDisplacementMap(const Image& source, const DisplacementMapSettings& settings)
{
    Image out;
    if (!source.IsValid()) return out;

    out.width = source.width;
    out.height = source.height;
    out.pixels.resize(static_cast<size_t>(source.width) * source.height * 4);

    // strength 2.0 default -> mild contrast; 1.0 = unchanged
    const float contrast = std::max(settings.strength, 0.01f);

    for (int i = 0; i < source.width * source.height; ++i)
    {
        const uint8_t* p = &source.pixels[i * 4];
        float g = (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) / 255.0f;

        g = (g - 0.5f) * contrast + 0.5f;   // contrast around mid grey
        g = std::clamp(g, 0.0f, 1.0f);
        if (settings.invertY) g = 1.0f - g; // invert option

        const uint8_t v = static_cast<uint8_t>(g * 255.0f);
        uint8_t* o = &out.pixels[i * 4];
        o[0] = o[1] = o[2] = v;
        o[3] = 255;
    }
    return out;
}
