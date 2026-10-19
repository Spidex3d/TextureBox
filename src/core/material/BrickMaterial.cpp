#include "core/material/BrickMaterial.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

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

BrickMaps GenerateBrickMaps(const BrickSettings& s)
{
    BrickMaps m;
    const int W = s.size, H = s.size;
    for (Image* img : { &m.albedo, &m.height, &m.roughness, &m.ao })
    {
        img->width = W;
        img->height = H;
        img->pixels.resize((size_t)W * H * 4);
    }

    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x)
        {
            float u = (float)x / W * s.cols;
            float v = (float)y / H * s.rows;

            int row = (int)std::floor(v);
            if (row & 1) u += 0.5f;           // offset every other row

            int col = (int)std::floor(u);
            float fu = u - std::floor(u);     // 0..1 inside brick
            float fv = v - std::floor(v);

            // distance to nearest edge of the brick, in brick units
            float edge = std::min(std::min(fu, 1.0f - fu), std::min(fv, 1.0f - fv));

            // brick mask: 0 in mortar, 1 on brick, soft bevel between
            float mask = Smooth(s.mortar * 0.5f, s.mortar * 0.5f + s.bevel, edge);

            float id = Hash(col, row);
            float shade = 1.0f + (id - 0.5f) * 2.0f * s.variation;

            //float br = s.brickColor[0] * shade, bg = s.brickColor[1] * shade, bb = s.brickColor[2] * shade;
            float mr = s.mortarColor[0], mg = s.mortarColor[1], mb = s.mortarColor[2];

			// random per-brick variation
            
            float br = std::clamp(s.brickColor[0] * shade, 0.0f, 1.0f);
            float bg = std::clamp(s.brickColor[1] * shade, 0.0f, 1.0f);
            float bb = std::clamp(s.brickColor[2] * shade, 0.0f, 1.0f);
			// end random variation

            const int i = y * W + x;
            Put(m.albedo, i,
                mr + (br - mr) * mask,
                mg + (bg - mg) * mask,
                mb + (bb - mb) * mask);

            Put(m.height, i, mask, mask, mask);

            float rough = 1.0f - 0.25f * mask * (0.5f + id * 0.5f); // brick 0.75..1.0, mortar 1.0
            Put(m.roughness, i, rough, rough, rough);

            float ao = 0.4f + 0.6f * Smooth(0.0f, 0.25f, edge);
            Put(m.ao, i, ao, ao, ao);
        }
    return m;
}