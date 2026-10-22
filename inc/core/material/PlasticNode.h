#pragma once
#include "core/material/PlasticMaterial.h"
#include "core/NodeType.h"
#include <vector>

inline PlasticSettings BuildPlasticSettings(const std::vector<const Node*>& chain)
{
    PlasticSettings ps;

    for (auto it = chain.rbegin(); it != chain.rend(); ++it)
    {
        const Node* m = *it;
        switch (m->type)
        {
            // generic controls
        case NodeType::Scale:      ps.scale = (float)m->i0; break;   // 1..12
        case NodeType::Detail:     ps.detail = m->f0;       break;         // 0..1
        case NodeType::Depth:      ps.depth = m->f0;        break;          // 0..1
        case NodeType::Random:     ps.variation = m->f0;    break;      // 0..1
        case NodeType::Shininess:  ps.shininess = m->f0;    break;      // 0..1
        case NodeType::Mottle:     ps.mottle = m->f0;       break;

            // optional: reuse FacePitting as speckle amount for plastic
        case NodeType::FacePitting:
            ps.speckle = m->f0; // suggest UI range 0..0.5 for plastic
            break;

            // colors
        case NodeType::Color:
            ps.plasticColorA[0] = m->col[0];
            ps.plasticColorA[1] = m->col[1];
            ps.plasticColorA[2] = m->col[2];
            break;

        case NodeType::MortarColor:
            ps.plasticColorB[0] = m->col[0];
            ps.plasticColorB[1] = m->col[1];
            ps.plasticColorB[2] = m->col[2];
            break;

        default:
            break;
        }
    }

    return ps;
}