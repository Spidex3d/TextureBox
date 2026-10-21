#pragma once
#include "window/NodeEditorPanel.h"
#include "core/material/RockMaterial.h"
#include <vector>

inline RockSettings BuildRockSettings(const std::vector<const Node*>& chain)
{
    RockSettings rs;
    for (auto it = chain.rbegin(); it != chain.rend(); ++it)
    {
        const Node* m = *it;
        switch (m->type)
        {
        case NodeType::Scale:       rs.scale = (float)m->i0;  break;       // slider 1..12
        case NodeType::Cracks:      rs.cracks = m->f0;         break;       // slider 0..1
        case NodeType::Detail:      rs.detail = m->f0;         break;       // slider 0..1
        case NodeType::Depth:       rs.depth = m->f0;         break;
        case NodeType::Random:      rs.variation = m->f0;         break;
        case NodeType::Shininess:   rs.shininess = m->f0;         break;
        case NodeType::Color:
            rs.rockColorA[0] = m->col[0]; rs.rockColorA[1] = m->col[1]; rs.rockColorA[2] = m->col[2];
            break;
        case NodeType::MortarColor:
            rs.rockColorB[0] = m->col[0]; rs.rockColorB[1] = m->col[1]; rs.rockColorB[2] = m->col[2];
            break;
        default: break;
        }
    }
    return rs;
}