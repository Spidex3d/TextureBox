#include "core/NormalMap.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

Image GenerateNormalMap(const Image& source, const NormalMapSettings& settings)
{
    Image out;
    if (!source.IsValid()) return out;

    out.width = source.width;
    out.height = source.height;
    out.pixels.resize(source.width * source.height * 4);

    // 1) Convert source RGBA -> grayscale height [0..1]
    std::vector<float> height(source.width * source.height);
    for (int i = 0; i < source.width * source.height; ++i)
    {
        const uint8_t* p = &source.pixels[i * 4];
        float h = (0.299f * p[0] + 0.587f * p[1] + 0.114f * p[2]) / 255.0f;
        height[i] = h;
    }

    auto H = [&](int x, int y) -> float
    {
        if (settings.wrap)
        {
            x = (x % source.width + source.width) % source.width;
            y = (y % source.height + source.height) % source.height;
        }
        else
        {
            x = std::clamp(x, 0, source.width - 1);
            y = std::clamp(y, 0, source.height - 1);
        }
        return height[y * source.width + x];
    };

    // 2) Sobel gradient -> normal
    for (int y = 0; y < source.height; ++y)
    {
        for (int x = 0; x < source.width; ++x)
        {
            float dx =
                (H(x + 1, y - 1) + 2.0f * H(x + 1, y) + H(x + 1, y + 1)) -
                (H(x - 1, y - 1) + 2.0f * H(x - 1, y) + H(x - 1, y + 1));

            float dy =
                (H(x - 1, y + 1) + 2.0f * H(x, y + 1) + H(x + 1, y + 1)) -
                (H(x - 1, y - 1) + 2.0f * H(x, y - 1) + H(x + 1, y - 1));

            float nx = -dx * settings.strength;
            float ny = (settings.invertY ? dy : -dy) * settings.strength;
            float nz = 1.0f;

            float len = std::sqrt(nx * nx + ny * ny + nz * nz);
            if (len > 0.0f) { nx /= len; ny /= len; nz /= len; }

            uint8_t* o = &out.pixels[(y * source.width + x) * 4];
            o[0] = static_cast<uint8_t>((nx * 0.5f + 0.5f) * 255.0f);
            o[1] = static_cast<uint8_t>((ny * 0.5f + 0.5f) * 255.0f);
            o[2] = static_cast<uint8_t>((nz * 0.5f + 0.5f) * 255.0f);
            o[3] = 255;
        }
    }

    return out;
}