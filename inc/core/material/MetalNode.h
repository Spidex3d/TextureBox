#pragma once
#include "core/material/MetalMaterial.h"
#include "window/NodeEditorPanel.h"
#include <vector>

inline MetalSettings BuildMetalSettings(const std::vector<const Node*>& chain)
{
    MetalSettings ms;
    for (auto it = chain.rbegin(); it != chain.rend(); ++it)
    {
        const Node* m = *it;
        switch (m->type)
        {
        case NodeType::Scale:           ms.scale = (float)m->i0;      break;       // slider 1..12
        case NodeType::Rust:            ms.rust = m->f0;              break; 
        case NodeType::Scratches:       ms.scratches = m->f0;         break;        // slider 0..1
        case NodeType::Detail:          ms.detail = m->f0;            break;       // slider 0..1
        case NodeType::Depth:           ms.depth = m->f0;             break;
        case NodeType::Random:          ms.variation = m->f0;         break;
        case NodeType::Shininess:       ms.shininess = m->f0;         break;
        case NodeType::Color:
            ms.metalColorA[0] = m->col[0]; ms.metalColorA[1] = m->col[1]; ms.metalColorA[2] = m->col[2];
            break;
        

        default: break;
        }
    }
    return ms;
}