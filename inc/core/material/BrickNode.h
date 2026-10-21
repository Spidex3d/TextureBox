#pragma once
#include <vector>
#include "window/NodeEditorPanel.h"      // Node, NodeType (adjust to your header)
#include "core/material/BrickMaterial.h"

// Applies every brick-related modifier node in the chain to bs.
// chain is ordered nearest-to-output first (as built in Evaluate),
// so we iterate it in reverse (material -> output).
inline BrickSettings BuildBrickSettings(const std::vector<const Node*>& chain)
{
    BrickSettings bs;

    for (auto it = chain.rbegin(); it != chain.rend(); ++it)
    {
        const Node* m = *it;
        switch (m->type)
        {
        case NodeType::Size:
            bs.cols = m->i0;
            bs.rows = m->i1;
            break;
        case NodeType::Color:
            bs.brickColor[0] = m->col[0];
            bs.brickColor[1] = m->col[1];
            bs.brickColor[2] = m->col[2];
            break;
        case NodeType::MortarColor:
            bs.mortarColor[0] = m->col[0];
            bs.mortarColor[1] = m->col[1];
            bs.mortarColor[2] = m->col[2];
            break;
        case NodeType::Random:
            bs.variation = m->f0;
            break;
        case NodeType::Bevel:
            bs.bevel = m->f0;
            break;
        case NodeType::EdgeSoften:
            bs.edgeSoftMul = m->f0;
            break;
        case NodeType::FacePitting:
            bs.pitting = m->f0;
            break;
        case NodeType::Shininess:
            bs.shininess = m->f0;
            break;
        default:
            break;



        }
    }
    return bs;
}
