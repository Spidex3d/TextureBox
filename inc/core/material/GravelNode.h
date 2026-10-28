#pragma once
#include "window/NodeEditorPanel.h"
#include "core/material/GravelMaterial.h"
#include <vector>

inline GravelSettings BuildGravelSettings(const std::vector<const Node*>& chain)
{
    GravelSettings rs;
    for (auto it = chain.rbegin(); it != chain.rend(); ++it)
    {
        const Node* m = *it;
        switch (m->type)
        {
        case NodeType::Scale:           rs.scale = (float)m->i0;        break;       // slider 1..12
        case NodeType::Detail:          rs.detail = m->f0;              break;       // slider 0..1
        case NodeType::Depth:           rs.depth = m->f0;               break;
        case NodeType::Random:          rs.variation = m->f0;           break;
		case NodeType::FacePitting:     rs.pitting = m->f0;             break;
        case NodeType::Shininess:       rs.shininess = m->f0;           break;
        case NodeType::StoneCount:      rs.stoneCount = m->i0;          break;
        case NodeType::Irregularity:    rs.irregularity = m->f0;        break;
        case NodeType::Coverage:        rs.coverage = m->f0;            break;
        case NodeType::Veins:           rs.veins = m->f0;               break;
        case NodeType::Color:
            rs.gravelColorA[0] = m->col[0]; rs.gravelColorA[1] = m->col[1]; rs.gravelColorA[2] = m->col[2];
            break;
        case NodeType::MortarColor:
            rs.gravelColorB[0] = m->col[0]; rs.gravelColorB[1] = m->col[1]; rs.gravelColorB[2] = m->col[2];
            break;
        default: break;
        }
    }
    return rs;
}
