#pragma once
#include "window/NodeEditorPanel.h"
#include "core/material/DirtMaterial.h"
#include <vector>

inline DirtSettings BuildDirtSettings(const std::vector<const Node*>& chain)
{
    DirtSettings rs;
    for (auto it = chain.rbegin(); it != chain.rend(); ++it)
    {
        const Node* m = *it;
        switch (m->type)
        {
        case NodeType::Scale:       rs.scale = (float)m->i0;  break;       // slider 1..12
        case NodeType::Detail:      rs.detail = m->f0;         break;       // slider 0..1
        case NodeType::Depth:       rs.depth = m->f0;         break;
        case NodeType::Random:      rs.variation = m->f0;         break;
        case NodeType::Shininess:   rs.shininess = m->f0;         break;
		case NodeType::DirtAmount:    rs.dirtAmount = m->i0;       break;
		case NodeType::ClodSize:    rs.clodSize = m->f0;       break;
        case NodeType::Color:
            rs.dirtColorA[0] = m->col[0]; rs.dirtColorA[1] = m->col[1]; rs.dirtColorA[2] = m->col[2];
            break;
        case NodeType::MortarColor:
            rs.dirtColorB[0] = m->col[0]; rs.dirtColorB[1] = m->col[1]; rs.dirtColorB[2] = m->col[2];
            break;
        default: break;
        }
    }
    return rs;
}