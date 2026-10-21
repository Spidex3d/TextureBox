#include <core/material/MetalMaterial.h>
#include <algorithm>
#include <cmath>



Maps GenerateMetalMaps(const MetalSettings& s)
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

    // tileable value noise
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

    // tileable fbm (period doubles per octave)
    auto fbmPeriodic = [&](float x, float y, int basePeriodX, int basePeriodY) -> float
    {
        float sum = 0.0f;
        float amp = 0.5f;

        float xf = x;
        float yf = y;

        int px = std::max(1, basePeriodX);
        int py = std::max(1, basePeriodY);

        for (int o = 0; o < 4; ++o)
        {
            sum += vnoisePeriodic(xf, yf, px, py) * amp;

            xf *= 2.0f;
            yf *= 2.0f;
            amp *= 0.5f;

            px *= 2;
            py *= 2;
        }

        return sum; // ~0..0.94
    };

    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; ++x)
        {
            float u = (float)x / (float)W;
            float v = (float)y / (float)H;

            // derive tile period from scale
            int pBase = std::max(1, (int)std::round(s.scale * 8.0f));

            float big = fbmPeriodic(
                u * (float)pBase,
                v * (float)pBase,
                pBase, pBase
            );

            float fine = fbmPeriodic(
                u * (float)(pBase * 6) + 20.0f,
                v * (float)(pBase * 6) + 7.0f,
                pBase * 6, pBase * 6
            );

			// scratches: thin dark lines where noise is near threshold
            int scratchPeriod = std::max(1, (pBase * 3) / 2);
            float cn = fbmPeriodic(
                u * (float)scratchPeriod + 25.0f,
                v * (float)scratchPeriod + 15.0f,
                scratchPeriod, scratchPeriod
            );

            float metal = big * (1.0f - s.detail * 0.5f) + fine * (s.detail * 0.5f);
            metal = Clamp01(metal * 1.2f);

			// Patches of rust: dark areas where noise is low
			float rustNoise = fbmPeriodic(
				u * (float)(pBase * 2) + 10.0f,
				v * (float)(pBase * 2) + 5.0f,
				pBase * 2, pBase * 2
			);
			rustNoise = Clamp01(rustNoise * 1.2f);
			rustNoise = 1.0f - rustNoise; // invert to get dark areas
			rustNoise = Smooth(0.0f, 0.5f, rustNoise); // smooth threshold

			// Scratches: thin dark lines where noise is near threshold
			float scratch = 1.0f - Smooth(0.0f, 0.04f, std::fabs(cn - 0.47f));
            scratch *= s.scratches;


            // cracks: thin dark lines where noise is near threshold
            float crack = 1.0f - Smooth(0.0f, 0.04f, std::fabs(cn - 0.47f));
            crack *= s.scratches;

            // height
            float h = metal * s.depth + (1.0f - s.depth) * 0.5f;
            h -= crack * 0.35f;
            h = Clamp01(h);

            // albedo
            float shade = 1.0f + (fine - 0.5f) * s.variation;
            float r = (s.metalColorB[0] + (s.metalColorA[0] - s.metalColorB[0]) * metal) * shade;
            float g = (s.metalColorB[1] + (s.metalColorA[1] - s.metalColorB[1]) * metal) * shade;
            float b = (s.metalColorB[2] + (s.metalColorA[2] - s.metalColorB[2]) * metal) * shade;

            float darken = 1.0f - crack * 0.6f;
            r = Clamp01(r * darken);
            g = Clamp01(g * darken);
            b = Clamp01(b * darken);

            const int i = y * W + x;
            Put(m.albedo, i, r, g, b);
            Put(m.height, i, h, h, h);
           // Put(m.metallic, i, rustNoise, rustNoise, rustNoise);

            // roughness
            float rough = 0.25f - 0.10f * metal;
            float wet = 0.15f + 0.10f * fine;
            rough = rough + (wet - rough) * s.shininess * (1.0f - crack);
            rough = Clamp01(rough);
            Put(m.roughness, i, rough, rough, rough);

            // AO
            float ao = 0.55f + 0.45f * metal - crack * 0.4f;
            ao = Clamp01(ao);
            Put(m.ao, i, ao, ao, ao);
        }
    }

    return m;
}
