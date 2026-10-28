#include "core/material/GravelMaterial.h"
#include <algorithm>
#include <cmath>

Maps GenerateGravelMaps(const GravelSettings& s)
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
        float sum = 0.0f, amp = 0.5f;
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

    // controls
    const int stones = std::max(4, s.stoneCount);
    const float veins = Clamp01(s.veins);
    const float detail = Clamp01(s.detail);
    const float depth = Clamp01(s.depth);
    const float var = Clamp01(s.variation);
    const float shine = Clamp01(s.shininess);

    // cell frequency derived from stone count
    const int cellN = std::max(2, (int)std::round(std::sqrt((float)stones)));
    const int pBase = std::max(1, (int)std::round(s.scale * 8.0f));

    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; ++x)
        {
            float u = (float)x / (float)W;
            float v = (float)y / (float)H;

            // base soil/noise under stones
            float nBig = fbmPeriodic(u * pBase, v * pBase, pBase, pBase);
            float nFine = fbmPeriodic(u * (pBase * 6) + 17.0f, v * (pBase * 6) + 9.0f, pBase * 6, pBase * 6);

            // Voronoi-like nearest stone center (tileable)
           // #####################

            const float irr = Clamp01(s.irregularity);
            const float cov = Clamp01(s.coverage);

            // tileable domain warp so cell edges become wobbly (not straight/round)
            float wx = fbmPeriodic(u * 6.0f + 3.0f, v * 6.0f + 8.0f, 6, 6) - 0.47f;
            float wy = fbmPeriodic(u * 6.0f + 11.0f, v * 6.0f + 2.0f, 6, 6) - 0.47f;

            float cu = u * (float)cellN + wx * irr * 1.2f;
            float cv = v * (float)cellN + wy * irr * 1.2f;
            int ci = (int)std::floor(cu);
            int cj = (int)std::floor(cv);
            float fx = cu - (float)ci;
            float fy = cv - (float)cj;

            float d1 = 1e9f, d2 = 1e9f;
            float sid = 0.0f;
            int sgx = 0, sgy = 0;

            for (int oy = -1; oy <= 1; ++oy)
            {
                for (int ox = -1; ox <= 1; ++ox)
                {
                    int gx = imod(ci + ox, cellN);
                    int gy = imod(cj + oy, cellN);

                    float jx = Hash(gx * 37 + 11, gy * 19 + 5);
                    float jy = Hash(gx * 13 + 7, gy * 29 + 3);

                    float dx = ((float)ox + 0.15f + 0.70f * jx) - fx;
                    float dy = ((float)oy + 0.15f + 0.70f * jy) - fy;

                    // per-stone rotation + stretch -> elongated, angular stones
                    float ang = Hash(gx * 7 + 1, gy * 5 + 9) * 6.2831853f;
                    float aspect = 1.0f + irr * 1.2f * Hash(gx * 3 + 4, gy * 11 + 2);
                    float ca = std::cos(ang), sa = std::sin(ang);
                    float rx = (dx * ca + dy * sa) / aspect;
                    float ry = (-dx * sa + dy * ca) * aspect;

                    // blend between round (euclid) and angular (chebyshev-ish) distance
                    float eu = std::sqrt(rx * rx + ry * ry);
                    float ch = std::max(std::fabs(rx), std::fabs(ry)) * 1.15f;
                    float d = eu + (ch - eu) * irr * 0.7f;

                    if (d < d1) { d2 = d1; d1 = d; sid = Hash(gx, gy); sgx = gx; sgy = gy; }
                    else if (d < d2) { d2 = d; }
                }
            }

            // cobble pavement: cells that fill the space (the look you liked)
            float cobble = Smooth(0.02f, 0.10f, d2 - d1);

            // gravel on top: lumpy outline, per-stone size, hidden by coverage
            float lump = vnoisePeriodic(u * (cellN * 8) + sid * 13.0f,
                v * (cellN * 8) + sid * 7.0f,
                cellN * 8, cellN * 8) - 0.5f;
            float stoneRadius = 0.20f + 0.10f * sid + lump * 0.12f * irr;
            float stoneMask = 1.0f - Smooth(stoneRadius * 0.85f, stoneRadius, d1);
            stoneMask *= (sid < cov) ? 1.0f : 0.0f;   // per-stone on/off
            stoneMask = Clamp01(stoneMask);

            float border = Clamp01((d2 - d1) * 3.5f);
            
            
            
            //float cu = u * (float)cellN;
            //float cv = v * (float)cellN;
            //int ci = (int)std::floor(cu);
            //int cj = (int)std::floor(cv);
            //float fx = cu - (float)ci;
            //float fy = cv - (float)cj;

            //float d1 = 1e9f, d2 = 1e9f;
            //float sid = 0.0f;
            //int sgx = 0, sgy = 0;

            //for (int oy = -1; oy <= 1; ++oy)
            //{
            //    for (int ox = -1; ox <= 1; ++ox)
            //    {
            //        int gx = imod(ci + ox, cellN);
            //        int gy = imod(cj + oy, cellN);

            //        float jx = Hash(gx * 37 + 11, gy * 19 + 5);
            //        float jy = Hash(gx * 13 + 7, gy * 29 + 3);

            //        float px = (float)ox + (0.15f + 0.70f * jx);
            //        float py = (float)oy + (0.15f + 0.70f * jy);

            //        float dx = px - fx;
            //        float dy = py - fy;
            //        float d = std::sqrt(dx * dx + dy * dy);

            //        if (d < d1)
            //        {
            //            d2 = d1;
            //            d1 = d;
            //            sid = Hash(gx, gy);
            //            sgx = gx; sgy = gy;
            //        }
            //        else if (d < d2)
            //        {
            //            d2 = d;
            //        }
            //    }
            //}

            //// stone mask
            //float stoneRadius = 0.22f + 0.10f * sid; // varied stone size
            //float stoneMask = 1.0f - Smooth(stoneRadius * 0.92f, stoneRadius, d1);
            //stoneMask = Clamp01(stoneMask);


            //#########
            // 
            // edge/rim between stones (for crevice AO + height dips)
            //float border = Clamp01((d2 - d1) * 3.5f);

            // per-stone micro noise
            float local = fbmPeriodic(
                u * (cellN * 3) + sid * 23.0f,
                v * (cellN * 3) + sid * 31.0f,
                cellN * 3, cellN * 3);

            // subtle veins inside stones
            float vn = vnoisePeriodic(
                u * (cellN * 10) + (float)sgx * 2.3f,
                v * (cellN * 10) + (float)sgy * 1.7f,
                cellN * 10, cellN * 10);
            float veinLines = 1.0f - Smooth(0.0f, 0.035f, std::fabs(vn - 0.5f));
            veinLines *= veins * stoneMask;

            // Height
            //float baseSoil = 0.42f + (nBig - 0.5f) * 0.10f;
            // becomes
            float baseSoil = 0.30f + cobble * 0.18f + (nBig - 0.5f) * 0.06f;
            
            
            float stoneH = 0.58f + (local - 0.5f) * 0.20f;
            float h = baseSoil + (stoneH - baseSoil) * stoneMask;
            h -= (1.0f - border) * 0.08f;            // crevice dip
            h += veinLines * 0.03f;                  // raised vein tiny
            h = 0.5f + (h - 0.5f) * (0.5f + depth);  // depth control
            h = Clamp01(h);

            // Albedo: soil + stones
            float soilTone = (0.30f + 0.30f * nFine) * (0.6f + 0.4f * cobble) + (sid - 0.5f) * var * 0.5f;
            soilTone = Clamp01(soilTone);


            //float soilTone = 0.35f + 0.25f * nFine;
            float stoneTone = 0.45f + 0.35f * local + (sid - 0.5f) * var;
            //stoneTone = Clamp01(stoneTone);

            float sr = s.gravelColorB[0] + (s.gravelColorA[0] - s.gravelColorB[0]) * soilTone;
            float sg = s.gravelColorB[1] + (s.gravelColorA[1] - s.gravelColorB[1]) * soilTone;
            float sb = s.gravelColorB[2] + (s.gravelColorA[2] - s.gravelColorB[2]) * soilTone;

            float rr = s.gravelColorB[0] + (s.gravelColorA[0] - s.gravelColorB[0]) * stoneTone;
            float rg = s.gravelColorB[1] + (s.gravelColorA[1] - s.gravelColorB[1]) * stoneTone;
            float rb = s.gravelColorB[2] + (s.gravelColorA[2] - s.gravelColorB[2]) * stoneTone;

            // blend soil->stone
            float r = sr + (rr - sr) * stoneMask;
            float g = sg + (rg - sg) * stoneMask;
            float b = sb + (rb - sb) * stoneMask;

            // veins brighten/desaturate slightly
            r = Clamp01(r + veinLines * 0.10f);
            g = Clamp01(g + veinLines * 0.10f);
            b = Clamp01(b + veinLines * 0.10f);

            // detail variation
            float shade = 1.0f + (nFine - 0.5f) * detail * 0.5f;
            r = Clamp01(r * shade);
            g = Clamp01(g * shade);
            b = Clamp01(b * shade);

            // Roughness
            float roughSoil = 0.88f - 0.05f * nFine;
            float roughStone = 0.72f - 0.12f * local;
            float rough = roughSoil + (roughStone - roughSoil) * stoneMask;
            rough += (1.0f - border) * 0.08f;          // crevices rougher
            rough = rough + ((0.22f + 0.08f * nFine) - rough) * shine; // wet control
            rough = Clamp01(rough);

            // AO
            float ao = 0.92f - (1.0f - border) * 0.30f; // darker in crevices
            ao -= stoneMask * 0.06f;                    // slight contact darkening
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
