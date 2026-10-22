#include <core/material/PlasticMaterial.h>
#include <algorithm>
#include <cmath>

Maps GeneratePlasticMaps(const PlasticSettings& s)
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
        return ab + (cd - ab) * fy;
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

    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; ++x)
        {
            float u = (float)x / (float)W;
            float v = (float)y / (float)H;

            float big = fbmPeriodic(u * (float)pBase, v * (float)pBase, pBase, pBase);
            float fine = fbmPeriodic(u * (float)(pBase * 5) + 19.0f,
                v * (float)(pBase * 5) + 11.0f,
                pBase * 5, pBase * 5);

            //float body = big * (1.0f - s.detail * 0.6f) + fine * (s.detail * 0.6f);
            //body = Clamp01(body * 1.15f);

            float bodyRaw = big * (1.0f - s.detail * 0.6f) + fine * (s.detail * 0.6f);
            bodyRaw = Clamp01(bodyRaw * 1.15f);

            float body = 0.5f + (bodyRaw - 0.5f) * s.mottle; // 0 = flat color, 1 = full mottling
            body = Clamp01(body);

            // tiny pigment speckle mask
            float spN = vnoisePeriodic(u * 512.0f + 3.0f, v * 512.0f + 7.0f, 512, 512);
            float speck = (1.0f - Smooth(0.0f, 0.06f, std::fabs(spN - 0.5f))) * s.speckle;

            // subtle molded flow lines
            float flowN = vnoisePeriodic(u * 7.0f + 2.0f, v * 120.0f + 17.0f, 7, 120);
            float flow = (1.0f - Smooth(0.0f, 0.03f, std::fabs(flowN - 0.5f))) * 0.08f;

            // albedo
            float shade = 1.0f + (fine - 0.5f) * s.variation;
            float r = (s.plasticColorB[0] + (s.plasticColorA[0] - s.plasticColorB[0]) * body) * shade;
            float g = (s.plasticColorB[1] + (s.plasticColorA[1] - s.plasticColorB[1]) * body) * shade;
            float b = (s.plasticColorB[2] + (s.plasticColorA[2] - s.plasticColorB[2]) * body) * shade;

            // speckles and flow darken/lighten subtly
            r = Clamp01(r + speck * 0.08f - flow * 0.10f);
            g = Clamp01(g + speck * 0.08f - flow * 0.10f);
            b = Clamp01(b + speck * 0.08f - flow * 0.10f);

            // height (very subtle on plastic)
            float h = 0.5f + (body - 0.5f) * s.depth;
            h += (speck - 0.5f * s.speckle) * 0.04f;
            h -= flow * 0.12f;
            h = Clamp01(h);

            // roughness: driven by shininess + micro variation
            float matteBase = 0.78f - 0.18f * body;
            float glossyTarget = 0.06f + 0.06f * fine; // glossy plastic
            float rough = matteBase + (glossyTarget - matteBase) * s.shininess;

            rough += flow * 0.20f;     // flow lines slightly rougher
            rough -= speck * 0.10f;    // bright specks slightly smoother
            rough = Clamp01(rough);

            

            


            // AO
            float ao = 0.88f + 0.12f * body - flow * 0.18f;
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