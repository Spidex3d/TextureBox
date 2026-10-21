#pragma once
#include <vector>
#include "window/NodeEditorPanel.h"      // Node, NodeType (adjust to your header)
#include "core/material/WoodMaterial.h"

// Applies every wood-related modifier node in the chain to ws.
// chain is ordered nearest-to-output first (as built in Evaluate),
// so we iterate it in reverse (material -> output).
inline WoodSettings BuildWoodSettings(const std::vector<const Node*>& chain)
{
    WoodSettings ws;

    for (auto it = chain.rbegin(); it != chain.rend(); ++it)
    {
        const Node* m = *it;
        switch (m->type)
        {
        case NodeType::Planks:      ws.plankCount = (float)m->i0;       break;
        case NodeType::Rings:       ws.rings = (float)m->i0;            break;
        case NodeType::GrainScale:  ws.grainScale = m->i0 * 20.0f;      break;
        case NodeType::SeamWidth:   ws.seamWidth = m->f0 * 0.005f;      break;
        case NodeType::Random:      ws.variation = m->f0;               break;
        case NodeType::FacePitting: ws.pitting = m->f0;                 break;
        case NodeType::Shininess:   ws.shininess = m->f0;               break;

        case NodeType::Color:
            ws.woodColorA[0] = m->col[0];
            ws.woodColorA[1] = m->col[1];
            ws.woodColorA[2] = m->col[2];
            break;
        case NodeType::MortarColor: // reused as the dark tone
            ws.woodColorB[0] = m->col[0];
            ws.woodColorB[1] = m->col[1];
            ws.woodColorB[2] = m->col[2];
            break;

        default: break;
        }
    }
    return ws;
}
