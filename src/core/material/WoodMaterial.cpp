#include "core/material/WoodMaterial.h"
#include <algorithm>
#include <cmath>

Maps GenerateWoodMaps(const WoodSettings& s)
{
    Maps m;
    const int W = s.size, H = s.size;
    for (Image* img : { &m.albedo, &m.height, &m.roughness, &m.ao })
    {
        img->width = W;
        img->height = H;
        img->pixels.resize((size_t)W * H * 4);
    }

    auto vnoise = [&](float x, float y) -> float
    {
        int xi = (int)std::floor(x), yi = (int)std::floor(y);
        float fx = x - xi, fy = y - yi;
        fx = fx * fx * (3.0f - 2.0f * fx);
        fy = fy * fy * (3.0f - 2.0f * fy);

        float a = Hash(xi, yi);
        float b = Hash(xi + 1, yi);
        float c = Hash(xi, yi + 1);
        float d = Hash(xi + 1, yi + 1);

        float ab = a + (b - a) * fx;
        float cd = c + (d - c) * fx;
        return ab + (cd - ab) * fy;
    };

    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; ++x)
        {
            float u = (float)x / (float)W;
            float v = (float)y / (float)H;

            // plank id across U
            float pu = u * s.plankCount;
            int plank = (int)std::floor(pu);
            float fpu = pu - std::floor(pu);

            float pid = Hash(plank, 17);
            float toneJitter = 1.0f + (pid - 0.5f) * 2.0f * s.variation;

            // warp coordinates for organic rings
            float nWarp = vnoise(u * 4.0f + pid * 13.0f, v * 4.0f + pid * 7.0f);
            float wu = u + (nWarp - 0.5f) * s.warp;
            float wv = v + (nWarp - 0.5f) * s.warp * 0.6f;

            // ring pattern + fine grain
            float rings = 0.5f + 0.5f * std::sin((wu * s.rings + wv * 1.2f + pid * 10.0f) * 6.2831853f);
            float grain = vnoise(wu * s.grainScale, wv * (s.grainScale * 0.45f));
            float grainFine = vnoise(wu * s.grainScale * 2.5f + 10.0f, wv * s.grainScale * 1.1f + 3.0f);

            float wood = rings * 0.65f + grain * 0.25f + grainFine * 0.10f; // 0..1-ish
            wood = Clamp01(wood);

            // plank seam mask
            float seamEdge = std::min(fpu, 1.0f - fpu);

            float sw = std::max(s.seamWidth, 0.0001f);
            float seam = 1.0f - Smooth(sw * 0.5f, sw, seamEdge);

            // color blend
            float br = s.woodColorA[0] + (s.woodColorB[0] - s.woodColorA[0]) * wood;
            float bg = s.woodColorA[1] + (s.woodColorB[1] - s.woodColorA[1]) * wood;
            float bb = s.woodColorA[2] + (s.woodColorB[2] - s.woodColorA[2]) * wood;

            br = Clamp01(br * toneJitter);
            bg = Clamp01(bg * toneJitter);
            bb = Clamp01(bb * toneJitter);

            // subtle dark seam
            br *= (1.0f - seam * 0.35f);
            bg *= (1.0f - seam * 0.35f);
            bb *= (1.0f - seam * 0.35f);

            

            // height: ring relief + tiny pitting + seam dip
            float micro = (grainFine - 0.5f) * s.pitting;
            float h = wood * 0.75f + micro;
            h -= seam * 0.10f;
            h = Clamp01(h);

            const int i = y * W + x;
            Put(m.albedo, i, br, bg, bb);
            Put(m.height, i, h, h, h);

            // roughness: lower on "varnished" (shininess high)
            float baseR = 0.72f - 0.22f * wood;              // grain variation
            float varnished = 0.10f + 0.05f * grain;         // smooth target
            float rough = baseR + (varnished - baseR) * s.shininess;
            rough += seam * 0.08f;                           // seams slightly rougher
            rough = Clamp01(rough);
            Put(m.roughness, i, rough, rough, rough);

            // AO: mild seam/ring occlusion
            float ao = 1.0f - seam * 0.20f - (1.0f - wood) * 0.06f;
            ao = Clamp01(ao);
            Put(m.ao, i, ao, ao, ao);
        }
    }

    return m;
}