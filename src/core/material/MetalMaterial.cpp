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

    const int pBase = std::max(1, (int)std::round(s.scale * 8.0f));

    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; ++x)
        {
            float u = (float)x / (float)W;
            float v = (float)y / (float)H;

            float big = fbmPeriodic(u * pBase, v * pBase, pBase, pBase);
            float fine = fbmPeriodic(u * (pBase * 6) + 20.0f, v * (pBase * 6) + 7.0f, pBase * 6, pBase * 6);

            float metal = Clamp01((big * (1.0f - s.detail * 0.5f) + fine * (s.detail * 0.5f)) * 1.2f);

            // ---- scratches: stretched noise, thin contour lines ----
            // set 1: runs along X (slow in x, fast in y)
            float n1 = vnoisePeriodic(u * 6.0f + 3.0f, v * 192.0f, 6, 192);
            // set 2: runs along Y
            float n2 = vnoisePeriodic(u * 160.0f + 9.0f, v * 5.0f, 160, 5);
            // set 3: fine brushed streaks along X
            float n3 = vnoisePeriodic(u * 4.0f, v * 512.0f, 4, 512);

            float sc1 = 1.0f - Smooth(0.0f, 0.025f, std::fabs(n1 - 0.5f));
            float sc2 = 1.0f - Smooth(0.0f, 0.020f, std::fabs(n2 - 0.5f));
            float sc3 = 1.0f - Smooth(0.0f, 0.040f, std::fabs(n3 - 0.5f));

            // break the lines up so they don't run the full width
            float breakup = Smooth(0.45f, 0.65f, fbmPeriodic(u * 12.0f, v * 12.0f, 12, 12));
            float scratch = Clamp01((sc1 + sc2) * breakup + sc3 * 0.25f) * s.scratches;

            // ---- rust: low-frequency patches, edged with fine noise ----
            float rn = fbmPeriodic(u * (pBase / 2 + 1) + 10.0f, v * (pBase / 2 + 1) + 5.0f,
                pBase / 2 + 1, pBase / 2 + 1);
            float threshold = 1.0f - s.rust;                 // more rust -> lower threshold
            float rustMask = Smooth(threshold - 0.08f, threshold + 0.08f, rn + (fine - 0.5f) * 0.25f);
            rustMask = Clamp01(rustMask * (s.rust > 0.0f ? 1.0f : 0.0f));
            // scratches expose bare metal, so rust is thinner there
            rustMask *= (1.0f - scratch * 0.7f);

			// ---- Moss: low-frequency patches, edged with fine noise ----
			float mn = fbmPeriodic(u * (pBase / 2 + 1) + 15.0f, v * (pBase / 2 + 1) + 8.0f,
			pBase / 2 + 1, pBase / 2 + 1);
			float thresholdm = 1.0f - s.moss;                 // more moss -> lower threshold
            float mossMask = Smooth(thresholdm - 0.08f, thresholdm + 0.08f, mn + (fine - 0.5f) * 0.25f);
			mossMask = Clamp01(mossMask * (s.moss > 0.0f ? 1.0f : 0.0f));
			mossMask *= (1.0f - scratch * 0.7f);
            mossMask *= (1.0f - rustMask * 0.8f);

            // ---- albedo ----
            float shade = 1.0f + (fine - 0.5f) * s.variation;
            float r = (s.metalColorB[0] + (s.metalColorA[0] - s.metalColorB[0]) * metal) * shade;
            float g = (s.metalColorB[1] + (s.metalColorA[1] - s.metalColorB[1]) * metal) * shade;
            float b = (s.metalColorB[2] + (s.metalColorA[2] - s.metalColorB[2]) * metal) * shade;

            // scratches are lighter bare metal
            r += scratch * 0.25f; g += scratch * 0.25f; b += scratch * 0.25f;

            // rust colour, varied by fine noise
            float rv = 0.7f + 0.6f * fine;
            float rr = s.rustColor[0] * rv, rg = s.rustColor[1] * rv, rb = s.rustColor[2] * rv;
            r = r + (rr - r) * rustMask;
            g = g + (rg - g) * rustMask;
            b = b + (rb - b) * rustMask;
            r = Clamp01(r); g = Clamp01(g); b = Clamp01(b);

			// moss colour, varied by fine noise
			float mv = 0.7f + 0.6f * fine;
			float mr = s.mossColor[0] * mv, mg = s.mossColor[1] * mv, mb = s.mossColor[2] * mv;
			r = r + (mr - r) * mossMask;
			g = g + (mg - g) * mossMask;
			b = b + (mb - b) * mossMask;
			r = Clamp01(r); g = Clamp01(g); b = Clamp01(b);

            // ---- height: scratches cut in, rust is pitted/raised ----
            float h = metal * s.depth + (1.0f - s.depth) * 0.5f;
            h -= scratch * 0.15f;
            h += rustMask * (0.08f + (fine - 0.5f) * 0.12f);
            h = Clamp01(h);

            // ---- roughness: shiny metal, rough rust, scratches scatter light ----
            float rough = 0.30f - 0.15f * metal;
            float polished = 0.08f + 0.05f * fine;
            rough = rough + (polished - rough) * s.shininess;
            rough += scratch * 0.25f;
            rough = rough + (0.85f - rough) * rustMask;
            rough = Clamp01(rough);

            // ---- AO ----
            float ao = Clamp01(0.85f + 0.15f * metal - scratch * 0.15f - rustMask * 0.25f);

			// ---- moss: moss is green, rough, and slightly darker ----
            rough = rough + (0.92f - rough) * mossMask;  // moss is rough
            ao = Clamp01(ao - mossMask * 0.12f);      // slightly darker occlusion

            const int i = y * W + x;
            Put(m.albedo, i, r, g, b);
            Put(m.height, i, h, h, h);
            Put(m.roughness, i, rough, rough, rough);
            Put(m.ao, i, ao, ao, ao);
        }
    }


    return m;
}
