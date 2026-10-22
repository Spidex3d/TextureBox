#pragma once
#include "core/material/MetalMaterial.h"
#include "core/NodeType.h"
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
        case NodeType::Moss: ms.moss = m->f0; ms.mossColor[0] = m->col[0]; ms.mossColor[1] = m->col[1]; ms.mossColor[2] = m->col[2];

            ms.metalColorA[0] = m->col[0]; ms.metalColorA[1] = m->col[1]; ms.metalColorA[2] = m->col[2];
            break;
        //case NodeType::MortarColor:
        //    ms.metalColorB[0] = m->col[0]; ms.metalColorB[1] = m->col[1]; ms.metalColorB[2] = m->col[2]; // moss
        //    break;
        case NodeType::RustColor:
            ms.rustColor[0] = m->col[0];  ms.rustColor[1] = m->col[1]; ms.rustColor[2] = m->col[2];
            break;
		

            break;


        default: break;
        }
    }
    return ms;
}