#include "core/material/DirtMaterial.h"
#include <algorithm>
#include <cmath>

Maps GenerateDirtMaps(const DirtSettings& s)
{
    Maps m;
    const int W = s.size, H = s.size;
    for (Image* img : { &m.albedo, &m.height, &m.roughness, &m.ao })
    {
        img->width = W;
        img->height = H;
        img->pixels.resize((size_t)W * H * 4);
    }

    auto imod = [](int a, int mod) -> int
    {
        int r = a % mod;
        return (r < 0) ? (r + mod) : r;
    };

    auto vnoisePeriodic = [&](float x, float y, int periodX, int periodY) -> float
    {
        int xi = (int)std::floor(x);
        int yi = (int)std::floor(y);

        float fx = x - (float)xi;
        float fy = y - (float)yi;

        fx = fx * fx * (3.0f - 2.0f * fx);
        fy = fy * fy * (3.0f - 2.0f * fy);

        int x0 = imod(xi, periodX);
        int y0 = imod(yi, periodY);
        int x1 = imod(xi + 1, periodX);
        int y1 = imod(yi + 1, periodY);

        float a = Hash(x0, y0);
        float b = Hash(x1, y0);
        float c = Hash(x0, y1);
        float d = Hash(x1, y1);

        float ab = a + (b - a) * fx;
        float cd = c + (d - c) * fx;
        return ab + (cd - ab) * fy; // 0..1
    };

    auto fbmPeriodic = [&](float x, float y, int basePeriodX, int basePeriodY) -> float
    {
        float sum = 0.0f;
        float amp = 0.5f;
        float xf = x, yf = y;
        int px = std::max(1, basePeriodX);
        int py = std::max(1, basePeriodY);

        for (int o = 0; o < 4; ++o)
        {
            sum += vnoisePeriodic(xf, yf, px, py) * amp;
            xf *= 2.0f; yf *= 2.0f; amp *= 0.5f;
            px *= 2; py *= 2;
        }
        return sum; // ~0..0.94
    };

    const int pBase = std::max(1, (int)std::round(s.scale * 8.0f));

    // Clamp artist controls defensively
    const float detail = Clamp01(s.detail);
    const float depth = Clamp01(s.depth);
    const float variation = Clamp01(s.variation);
    const float colorVar = Clamp01(s.colorVariation);
    const float shininess = Clamp01(s.shininess);
    const float clodSize = std::clamp(s.clodSize, 0.02f, 0.95f);
    const int dirtAmt = std::max(1, s.dirtAmount);

    // derive clod frequency from dirtAmount
    const int clodFreq = std::max(2, (int)std::round(std::sqrt((float)dirtAmt)));

    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; ++x)
        {
            float u = (float)x / (float)W;
            float v = (float)y / (float)H;

            // Base soil undulation
            float nBig = fbmPeriodic(u * (float)pBase, v * (float)pBase, pBase, pBase);
            float nFine = fbmPeriodic(u * (float)(pBase * 6) + 21.0f,
                v * (float)(pBase * 6) + 9.0f,
                pBase * 6, pBase * 6);

            float soil = nBig * (1.0f - detail * 0.6f) + nFine * (detail * 0.6f);
            soil = Clamp01(soil * 1.12f);

            // Clod mask (tileable cellular-ish blobs)
            float cu = u * (float)clodFreq;
            float cv = v * (float)clodFreq;
            int ci = (int)std::floor(cu);
            int cj = (int)std::floor(cv);
            float fx = cu - (float)ci;
            float fy = cv - (float)cj;

            float minDist = 1e9f;
            float clodId = 0.0f;

            // check neighboring cells for nearest random center
            for (int oy = -1; oy <= 1; ++oy)
            {
                for (int ox = -1; ox <= 1; ++ox)
                {
                    int gx = imod(ci + ox, clodFreq);
                    int gy = imod(cj + oy, clodFreq);

                    float jx = Hash(gx * 31 + 7, gy * 17 + 3);
                    float jy = Hash(gx * 13 + 11, gy * 29 + 5);

                    float px = (float)ox + (jx * 0.7f + 0.15f); // center in neighbor cell space
                    float py = (float)oy + (jy * 0.7f + 0.15f);

                    float dx = px - fx;
                    float dy = py - fy;
                    float d2 = dx * dx + dy * dy;

                    if (d2 < minDist)
                    {
                        minDist = d2;
                        clodId = Hash(gx, gy);
                    }
                }
            }

            float d = std::sqrt(minDist); // ~0..1+
            float radius = 0.10f + clodSize * 0.45f; // clod size control
            float clodMask = 1.0f - Smooth(radius * 0.75f, radius, d);
            clodMask = Clamp01(clodMask);

            // breakup so clods blend naturally into soil
            float breakup = fbmPeriodic(u * 16.0f + 4.0f, v * 16.0f + 12.0f, 16, 16);
            clodMask *= Smooth(0.25f, 0.9f, breakup);

            // Height: soil base + raised clods - tiny pits
            float h = 0.5f + (soil - 0.5f) * depth;
            h += clodMask * (0.12f + 0.18f * depth);
            h -= (1.0f - clodMask) * (nFine - 0.5f) * 0.05f;
            h = Clamp01(h);

            // Albedo blend between dark and light dirt with variation
            float tone = Clamp01(soil * 0.75f + clodMask * 0.35f);
            float perClodShade = 1.0f + (clodId - 0.5f) * 2.0f * colorVar;
            float microShade = 1.0f + (nFine - 0.5f) * variation;

            float rBase = s.dirtColorB[0] + (s.dirtColorA[0] - s.dirtColorB[0]) * tone;
            float gBase = s.dirtColorB[1] + (s.dirtColorA[1] - s.dirtColorB[1]) * tone;
            float bBase = s.dirtColorB[2] + (s.dirtColorA[2] - s.dirtColorB[2]) * tone;

            float shade = perClodShade * microShade;
            float r = Clamp01(rBase * shade);
            float g = Clamp01(gBase * shade);
            float b = Clamp01(bBase * shade);

            // Roughness: dirt is rough; wetness from shininess lowers roughness
            float rough = 0.88f - 0.10f * soil - 0.08f * clodMask;
            float wetTarget = 0.25f + 0.12f * nFine;
            rough = rough + (wetTarget - rough) * shininess;
            rough = Clamp01(rough);

            // AO: depressions darker
            float ao = 0.60f + 0.40f * h;
            ao -= (1.0f - clodMask) * 0.10f;
            ao = Clamp01(ao);

            const int i = y * W + x;
            Put(m.albedo, i, r, g, b);
            Put(m.height, i, h, h, h);
            Put(m.roughness, i, rough, rough, rough);
            Put(m.ao, i, ao, ao, ao);
        }
    }

    return m;
}