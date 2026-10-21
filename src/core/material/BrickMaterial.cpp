#include "core/material/BrickMaterial.h"
#include <algorithm>
#include <cmath>

Maps GenerateBrickMaps(const BrickSettings& s)
{
    Maps m;
    const int W = s.size, H = s.size;
    for (Image* img : { &m.albedo, &m.height, &m.roughness, &m.ao })
    {
        img->width = W;
        img->height = H;
        img->pixels.resize((size_t)W * H * 4);
    }

    // tiny smooth value-noise helper
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
        return ab + (cd - ab) * fy; // 0..1
    };

    for (int y = 0; y < H; ++y)
    {
        for (int x = 0; x < W; ++x)
        {
            float u = (float)x / W * s.cols;
            float v = (float)y / H * s.rows;

            int row = (int)std::floor(v);
            if (row & 1) u += 0.5f; // running bond offset

            int col = (int)std::floor(u);
            float fu = u - std::floor(u);
            float fv = v - std::floor(v);

            // distance to nearest brick edge
            float edge = std::min(std::min(fu, 1.0f - fu), std::min(fv, 1.0f - fv));

            // base brick mask (0 mortar -> 1 brick) with soft bevel band
            float mask = Smooth(s.mortar * 0.5f, s.mortar * 0.5f + s.bevel, edge);

            float id = Hash(col, row); // per-brick random
            float shade = 1.0f + (id - 0.5f) * 2.0f * s.variation;

            
            // colors
            float br = std::clamp(s.brickColor[0] * shade, 0.0f, 1.0f);
            float bg = std::clamp(s.brickColor[1] * shade, 0.0f, 1.0f);
            float bb = std::clamp(s.brickColor[2] * shade, 0.0f, 1.0f);

            float mr = s.mortarColor[0];
            float mg = s.mortarColor[1];
            float mb = s.mortarColor[2];

            // -------- Height shaping --------
            // Wider, smoother edge rolloff (bevel softening)
            //float edgeSoft = Smooth(s.mortar * 0.45f, s.mortar * 0.45f + s.bevel * 1.6f, edge);
            float edgeSoft = Smooth(s.mortar * 0.45f, s.mortar * 0.45f + s.bevel * s.edgeSoftMul, edge);

            // brick face micro variation (subtle pitting)
            float n1 = vnoise(u * 7.0f + id * 11.0f, v * 7.0f + id * 13.0f);   // medium
            float n2 = vnoise(u * 18.0f + id * 3.0f, v * 18.0f + id * 5.0f);   // fine
            
            // new bit
            float gloss = 1.0f + 0.06f * s.shininess * (n1 - 0.5f) * mask;
            br = std::clamp(br * gloss, 0.0f, 1.0f);
            bg = std::clamp(bg * gloss, 0.0f, 1.0f);
            bb = std::clamp(bb * gloss, 0.0f, 1.0f);




            //float micro = (n1 * 0.65f + n2 * 0.35f - 0.5f) * 0.08f;            // ~[-0.04, +0.04]
            float micro = (n1 * 0.65f + n2 * 0.35f - 0.5f) * s.pitting;

            // final height: mortar low, brick high, softened near edges
            float h = edgeSoft;                 // 0..1 (main shape)
            h += micro * edgeSoft;              // noise only on brick area
            h = std::clamp(h, 0.0f, 1.0f);

            // albedo (still uses main mask)
            const int i = y * W + x;
            Put(m.albedo, i,
                mr + (br - mr) * mask,
                mg + (bg - mg) * mask,
                mb + (bb - mb) * mask);

            // height map output
            Put(m.height, i, h, h, h);

            // roughness: brick less rough than mortar + tiny variation
            float rough = 1.0f - 0.28f * mask * (0.55f + id * 0.45f);
            rough += (n2 - 0.5f) * 0.05f * mask;

            // shininess: glazed face is smooth, mortar stays rough
            float glossy = 0.08f + 0.04f * n1;                 // very smooth, slight variation
            rough = rough + (glossy - rough) * s.shininess * mask;
            rough = std::clamp(rough, 0.0f, 1.0f);
            Put(m.roughness, i, rough, rough, rough);

            // roughness: brick less rough than mortar + tiny variation
            /*float rough = 1.0f - 0.28f * mask * (0.55f + id * 0.45f);
            rough += (n2 - 0.5f) * 0.05f * mask;
            rough = std::clamp(rough, 0.0f, 1.0f);
            Put(m.roughness, i, rough, rough, rough);*/

            // AO stronger in joints and near edges
            float joint = 1.0f - edgeSoft; // high in mortar/joint
            float ao = 1.0f - (0.45f * joint + 0.12f * (1.0f - mask));
            ao = std::clamp(ao, 0.0f, 1.0f);
            Put(m.ao, i, ao, ao, ao);
        }
    }

    return m;
}
