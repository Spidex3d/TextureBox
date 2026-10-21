#include "window/NodeEditorPanel.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <core/FileDialog.h>

#include "core/material/BrickMaterial.h"
//#include "core/material/MetalMaterial.h"
//#include "core/material/GlassMaterial.h"
//#include "core/material/ColorMaterial.h"

#include <Windows.h>

#include <imgui/imgui_internal.h>


const char* NodeEditorPanel::NodeTypeName(NodeType t)
{
    switch (t)
    {
    case NodeType::Output: return "Output";
    case NodeType::Noise: return "Noise";
    case NodeType::Blend: return "Blend";
    case NodeType::HeightToNormal: return "HeightToNormal";
    case NodeType::Roughness: return "Roughness";
    case NodeType::Color: return "Color";
    case NodeType::MortarColor: return "MortarColor";
    case NodeType::Albedo: return "Albedo";
	case NodeType::Depth: return "Depth";
	case NodeType::Size: return "Size";
    case NodeType::AO: return "AO";
    case NodeType::Image: return "Image";
    case NodeType::Material: return "Material";
    case NodeType::Random: return "Random";
    default: return "Node";
    }
}
// zoom and pan math
ImVec2 NodeEditorPanel::CanvasToScreen(const ImVec2& p, const ImVec2& origin) const
{
    return ImVec2(
        origin.x + (p.x + m_panning.x) * m_zoom,
        origin.y + (p.y + m_panning.y) * m_zoom
    );
}

ImVec2 NodeEditorPanel::ScreenToCanvas(const ImVec2& p, const ImVec2& origin) const
{
    return ImVec2(
        (p.x - origin.x) / m_zoom - m_panning.x,
        (p.y - origin.y) / m_zoom - m_panning.y
    );
}

void NodeEditorPanel::EnsureBootstrapped()
{
    if (!m_nodes.empty()) return;

    NewNode(NodeType::Output, ImVec2(400, 120));

}

int NodeEditorPanel::NewNode(NodeType type, const ImVec2& pos)
{
    Node n;
    n.id = m_nextNodeId++;
    n.type = type;
    n.name = NodeTypeName(type);
    n.pos = pos;

    //---------------------------------------------------------------
	// set default size based on node type
	// ---------------------------------------------------------------
    switch (type)
    {
    case NodeType::Output:         n.size = ImVec2(190, 200);  break;
    case NodeType::Image:          n.size = ImVec2(200, 120);  break;
    case NodeType::Material:       n.size = ImVec2(200, 120);  break;
    case NodeType::Random:         n.size = ImVec2(200, 80);  break;
    case NodeType::Noise:          n.size = ImVec2(200, 80);  break;
    case NodeType::Blend:          n.size = ImVec2(200, 80);  break;
    case NodeType::Albedo:         n.size = ImVec2(80, 80);  break;
    case NodeType::HeightToNormal: n.size = ImVec2(200, 90);  break;
    case NodeType::Roughness:      n.size = ImVec2(200, 90);  break;
    case NodeType::AO:             n.size = ImVec2(200, 90);  break;
	case NodeType::Color:          n.size = ImVec2(200, 70);  break;
	case NodeType::MortarColor:     n.size = ImVec2(200, 70);  break;
	case NodeType::Depth:          n.size = ImVec2(200, 80);  break;
	case NodeType::Size:           n.size = ImVec2(200, 80);  break;

    }
       
    if (type == NodeType::AO) { n.f0 = 1.5f; n.i0 = 0; }

    BuildPinsForNode(n);
    m_nodes.push_back(n);
    return n.id;
}


void NodeEditorPanel::BuildPinsForNode(Node& n)
{
    n.inputs.clear();
    n.outputs.clear();
    //##
    auto addIn = [&](const char* name, PinKind kind) {
        Pin p; p.id = m_nextPinId++; p.nodeId = n.id; p.isInput = true; p.name = name; p.kind = kind;
        n.inputs.push_back(p);
    };
    auto addOut = [&](const char* name, PinKind kind) {
        Pin p; p.id = m_nextPinId++; p.nodeId = n.id; p.isInput = false; p.name = name; p.kind = kind;
        n.outputs.push_back(p);
    };
    
    switch (n.type)
    {
        // we need to have load at start up
    case NodeType::Output:
        addIn("Albedo", PinKind::Surface);
        addIn("Normal", PinKind::Image);
		addIn("Roughness", PinKind::Image);
		addIn("Displacement", PinKind::Image);
		addIn("Specular", PinKind::Image);
        addIn("AO", PinKind::Image);
        break;
    case NodeType::Albedo:
		addIn("Img", PinKind::Image);
        addIn("Mat ", PinKind::Material);
		addOut("out", PinKind::Surface);
        break;
    case NodeType::Image:
        addOut("out", PinKind::Image);
        break;
    case NodeType::Material:
        addOut("out", PinKind::Material);
        break;
        
    case NodeType::Random:
        addIn("in", PinKind::Material);
        addOut("out", PinKind::Material);
        break;
    
    case NodeType::Noise:
        addOut("Value", PinKind::Scalar);
        break;
    case NodeType::Color:
        addIn("in", PinKind::Material);
        addOut("out", PinKind::Material);
        break;
    case NodeType::MortarColor:
		addIn("in", PinKind::Material);
        addOut("out", PinKind::Material);
        break;
    case NodeType::Blend:
        addIn("A", PinKind::Material);
        addIn("B", PinKind::Material);
        addOut("Out", PinKind::Material);
        break;
	case NodeType::Depth:   
		addIn("in", PinKind::Scalar);
        addOut("out", PinKind::Scalar);
        break;
    case NodeType::Size:
        addIn("in", PinKind::Material);
        addOut("out", PinKind::Material);
        break;
	
    case NodeType::HeightToNormal:
        addIn("Height", PinKind::Image);
        addOut("Normal", PinKind::Image);
        break;
	case NodeType::Roughness:
		addIn("Image", PinKind::Image);
		addOut("Roughness", PinKind::Image);
		break;
    case NodeType::AO:
        addIn("Image", PinKind::Image);
        addOut("AO", PinKind::Image);
        break;
    
    }
}

const NodeEditorPanel::Node* NodeEditorPanel::FindUpstream(int inputPinId) const
{
    for (const auto& l : m_links)
    {
        if (l.toPinId != inputPinId) continue;
        for (const auto& n : m_nodes)
            for (const auto& p : n.outputs)
                if (p.id == l.fromPinId) return &n;
    }
    return nullptr;
}

const NodeEditorPanel::Node* NodeEditorPanel::ResolveAlbedoSource(const Node* n) const
{
    // Albedo node: look through to Material (preferred) or Image
    if (n && n->type == NodeType::Albedo && n->inputs.size() >= 2)
    {
        const Node* viaMaterial = FindUpstream(n->inputs[0].id);
        const Node* viaImage = FindUpstream(n->inputs[1].id);
        return viaMaterial ? viaMaterial : viaImage;
    }
    return n; // already a direct source
}

void NodeEditorPanel::Evaluate(TextureDocument& doc)
{
    // find first Output node
    const Node* out = nullptr;
    for (const auto& n : m_nodes)
        if (n.type == NodeType::Output) { out = &n; break; }

    if (!out || out->inputs.size() < 2) return;

    
    if (!out)
    {
        const int newOutId = NewNode(NodeType::Output, ImVec2(640, 110));
        out = FindNode(newOutId);
        if (!out || out->inputs.size() < 2) return;
    }

    doc.materialName = out->matName;

    // Albedo input
    //const Node* albedoSrc = FindUpstream(out->inputs[0].id);
    const Node* albedoSrc = ResolveAlbedoSource(FindUpstream(out->inputs[0].id));
    const Node* normalNode = FindUpstream(out->inputs[1].id);
	//const Node* roughnessNode = FindUpstream(out->inputs[2].id);    

    // 1) Load albedo image if connected
    if (albedoSrc && albedoSrc->type == NodeType::Image && albedoSrc->path[0] != '\0')
    {
        doc.LoadSource(albedoSrc->path);
    }

    // 2) Generate normal if Output.Normal is fed by HeightToNormal <- Image
    if (normalNode && normalNode->type == NodeType::HeightToNormal && !normalNode->inputs.empty())
    {
        const Node* heightSrc = FindUpstream(normalNode->inputs[0].id);
        if (heightSrc && heightSrc->type == NodeType::Image && heightSrc->path[0] != '\0')
        {
            // if height image differs from albedo image, load that before generating normal
            if (!albedoSrc || strcmp(heightSrc->path, albedoSrc->path) != 0)
                doc.LoadSource(heightSrc->path);

            if (doc.sourceLoaded)
            {
                // Apply node strength to document settings
                doc.normalSettings.strength = normalNode->f0;

                // TODO: call your existing normal build function here.
                // Replace this with whatever your app currently uses, e.g.:
                // BuildNormalMap(doc.sourceImage, doc.normalSettings, doc.normalImage);
                // doc.normalReady = doc.normalTex.Upload(doc.normalImage);

                if (doc.sourceLoaded)
                {
                    // apply node strength to the document settings
                    doc.normalSettings.strength = normalNode->f0;

                    // generate and upload the normal map
                    doc.normalImage = GenerateNormalMap(doc.sourceImage, doc.normalSettings);
                    doc.normalTex.Destroy();
                    doc.normalReady = doc.normalImage.IsValid() && doc.normalTex.Upload(doc.normalImage);
                }
            }
        }
    }

    // 3) Roughness: Output.Roughness <- Roughness <- Image
    if (out->inputs.size() >= 3)
    {
        const Node* roughNode = FindUpstream(out->inputs[2].id);
        if (roughNode && roughNode->type == NodeType::Roughness && !roughNode->inputs.empty())
        {
            const Node* src = FindUpstream(roughNode->inputs[0].id);
            if (src && src->type == NodeType::Image && src->path[0] != '\0')
            {
                // LoadSource wipes normal + roughness, so only reload if needed
                if (doc.sourcePath != src->path || !doc.sourceLoaded)
                    doc.LoadSource(src->path);

                if (doc.sourceLoaded)
                {
                    doc.roughnessSettings.strength = roughNode->f0;
                    doc.roughnessSettings.invertY = roughNode->i0 != 0;

                    doc.roughnessImage = GenerateRoughnessMap(doc.sourceImage, doc.roughnessSettings);
                    doc.roughnessTex.Destroy();
                    doc.roughnessReady = doc.roughnessImage.IsValid() && doc.roughnessTex.Upload(doc.roughnessImage);
                }
            }
        }
    }

    // 4) AO: Output.AO <- AO <- Image
    if (out->inputs.size() >= 4)
    {
        const Node* aoNode = FindUpstream(out->inputs[3].id);
        if (aoNode && aoNode->type == NodeType::AO && !aoNode->inputs.empty())
        {
            const Node* src = FindUpstream(aoNode->inputs[0].id);
            if (src && src->type == NodeType::Image && src->path[0] != '\0')
            {
                if (doc.sourcePath != src->path || !doc.sourceLoaded)
                    doc.LoadSource(src->path);

                if (doc.sourceLoaded)
                {
                    AOSettings s;
                    s.strength = aoNode->f0;
                    s.invert = (aoNode->i0 != 0);

                    doc.aoImage = GenerateAOMap(doc.sourceImage, s);
                    doc.aoTex.Destroy();
                    doc.aoReady = doc.aoImage.IsValid() && doc.aoTex.Upload(doc.aoImage);
                }
            }
        }
    }


    // -----------------------------------------------------------
    // in Evaluate(), before the image-based blocks
	// check for a Brick node connected to Output.Albedo
	// -----------------------------------------------------------
   
    // Material chain for Output.Albedo:
// Material -> (optional Size/Color/Random...) -> Output.Albedo
    {
        std::vector<const Node*> chain;
        const Node* cur = albedoSrc;

        // walk upstream until we reach the Material node
        for (int guard = 0; cur && cur->type != NodeType::Material && guard < 64; ++guard)
        {
            chain.push_back(cur);
            cur = cur->inputs.empty() ? nullptr : FindUpstream(cur->inputs[0].id);
        }

        // if we found a Material node, generate from it + modifiers
        if (cur && cur->type == NodeType::Material &&
            (MaterialKind)cur->materialKind == MaterialKind::Brick)
        {
            BrickSettings bs;
            bs.cols = 4;
            bs.rows = 4;

            // apply modifiers from nearest-to-material -> nearest-to-output
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
                default:
                    break;
                }
            }

            BrickMaps maps = GenerateBrickMaps(bs);

            doc.sourceImage = maps.albedo;
            doc.sourceTex.Destroy();
            doc.sourceLoaded = doc.sourceTex.Upload(doc.sourceImage);

            doc.normalSettings.strength = 4.0f;
            doc.normalImage = GenerateNormalMap(maps.height, doc.normalSettings);
            doc.normalTex.Destroy();
            doc.normalReady = doc.normalImage.IsValid() && doc.normalTex.Upload(doc.normalImage);

            doc.roughnessImage = maps.roughness;
            doc.roughnessTex.Destroy();
            doc.roughnessReady = doc.roughnessTex.Upload(doc.roughnessImage);

            return; // done for material chain path
        }
    }

}

NodeEditorPanel::Node* NodeEditorPanel::FindNode(int nodeId)
{
    for (auto& n : m_nodes) if (n.id == nodeId) return &n;
    return nullptr;
}

NodeEditorPanel::Pin* NodeEditorPanel::FindPin(int pinId)
{
    for (auto& n : m_nodes)
    {
        for (auto& p : n.inputs)  if (p.id == pinId) return &p;
        for (auto& p : n.outputs) if (p.id == pinId) return &p;
    }
    return nullptr;
}

bool NodeEditorPanel::IsPinConnectedAsInput(int pinId) const
{
    for (const auto& l : m_links)
        if (l.toPinId == pinId) return true;
    return false;
}

bool NodeEditorPanel::CanCreateLink(int fromPinId, int toPinId) const
{
    const Pin* from = nullptr;
    const Pin* to = nullptr;

    for (const auto& n : m_nodes)
    {
        for (const auto& p : n.inputs)
        {
            if (p.id == fromPinId) from = &p;
            if (p.id == toPinId)   to = &p;
        }
        for (const auto& p : n.outputs)
        {
            if (p.id == fromPinId) from = &p;
            if (p.id == toPinId)   to = &p;
        }
    }

    if (!from || !to) return false;
    if (from->isInput) return false;
    if (!to->isInput) return false;
    if (from->nodeId == to->nodeId) return false;
    if (from->kind != to->kind) return false;
    // one connection per input pin
    for (const auto& l : m_links)
        if (l.toPinId == toPinId) return false;

    // prevent exact duplicate
    for (const auto& l : m_links)
        if (l.fromPinId == fromPinId && l.toPinId == toPinId) return false;

    return true;
}

void NodeEditorPanel::TryCreateLink(int fromPinId, int toPinId)
{
    if (!CanCreateLink(fromPinId, toPinId)) return;

    Pin* to = FindPin(toPinId);
    if (!to) return;

    Node* toNode = FindNode(to->nodeId);
    if (!toNode) return;

    // Special rule: Albedo node has two alternative inputs (Material/Image).
    // When one is connected, remove any link on the other.
    if (toNode->type == NodeType::Albedo && toNode->inputs.size() >= 2)
    {
        const int materialIn = toNode->inputs[0].id; // "Material"
        const int imageIn = toNode->inputs[1].id; // "Image"

        int otherInput = -1;
        if (toPinId == materialIn) otherInput = imageIn;
        else if (toPinId == imageIn) otherInput = materialIn;

        if (otherInput != -1)
        {
            m_links.erase(
                std::remove_if(m_links.begin(), m_links.end(),
                    [&](const Link& l) { return l.toPinId == otherInput; }),
                m_links.end());
        }
    }

    Link l;
    l.id = m_nextLinkId++;
    l.fromPinId = fromPinId;
    l.toPinId = toPinId;
    m_links.push_back(l);
}

void NodeEditorPanel::RemoveLink(int linkId)
{
    m_links.erase(
        std::remove_if(m_links.begin(), m_links.end(),
            [=](const Link& l) { return l.id == linkId; }),
        m_links.end());
}

ImU32 NodeEditorPanel::PinColor(PinKind k, bool isInput)
{
    switch (k)
    {
    case PinKind::Material: return isInput ? IM_COL32(80, 170, 255, 255) : IM_COL32(80, 220, 120, 255); // blue in, green out
    case PinKind::Image:    return isInput ? IM_COL32(255, 90, 90, 255) : IM_COL32(255, 170, 60, 255); // red in, orange out
    case PinKind::Scalar:   return isInput ? IM_COL32(190, 120, 255, 255) : IM_COL32(210, 160, 255, 255);
    case PinKind::Color:    return isInput ? IM_COL32(255, 120, 220, 255) : IM_COL32(255, 180, 235, 255);
    }
    return IM_COL32(200, 200, 200, 255);
}

static float PinTopPad(int nodeType, int outputNodeType)
{
    // Output node: leave room for the Name box, then pins start lower
    return (nodeType == outputNodeType) ? 62.0f : 30.0f;
}

ImVec2 NodeEditorPanel::GetPinScreenPos(const Pin& pin, const ImVec2& origin) const
{
    const Node* node = nullptr;
    for (const auto& n : m_nodes) if (n.id == pin.nodeId) { node = &n; break; }
    if (!node) return origin;

    const float topPad = (node->type == NodeType::Output) ? 62.0f : 30.0f;
    const float step = 22.0f;

    if (pin.isInput)
    {
        int idx = 0;
        for (size_t i = 0; i < node->inputs.size(); ++i)
            if (node->inputs[i].id == pin.id) { idx = (int)i; break; }

        return CanvasToScreen(ImVec2(node->pos.x, node->pos.y + topPad + idx * step), origin);
    }
    else
    {
        int idx = 0;
        for (size_t i = 0; i < node->outputs.size(); ++i)
            if (node->outputs[i].id == pin.id) { idx = (int)i; break; }

        return CanvasToScreen(ImVec2(node->pos.x + node->size.x, node->pos.y + topPad + idx * step), origin);
    }
}
//ImVec2 NodeEditorPanel::GetPinScreenPos(const Pin& pin, const ImVec2& origin) const
//{
//    // pin layout rule:
//    // inputs on left, outputs on right, evenly spaced vertically in each side group
//    const Node* node = nullptr;
//    for (const auto& n : m_nodes) if (n.id == pin.nodeId) { node = &n; break; }
//    if (!node) return origin;
//
//    const float topPad = (node->type == NodeType::Output) ? 62.0f : 30.0f;
//    //const float topPad = 30.0f;
//    const float step = 22.0f;
//
//    if (pin.isInput)
//    {
//        int idx = 0;
//        for (size_t i = 0; i < node->inputs.size(); ++i) if (node->inputs[i].id == pin.id) { idx = (int)i; break; }
//        return ImVec2(origin.x + node->pos.x, origin.y + node->pos.y + topPad + idx * step);
//    }
//    else
//    {
//        int idx = 0;
//        for (size_t i = 0; i < node->outputs.size(); ++i) if (node->outputs[i].id == pin.id) { idx = (int)i; break; }
//        return ImVec2(origin.x + node->pos.x + node->size.x, origin.y + node->pos.y + topPad + idx * step);
//    }
//}

void NodeEditorPanel::DrawGrid(ImDrawList* dl, const ImVec2& canvasPos, const ImVec2& canvasSize)
{
    dl->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
        IM_COL32(26, 26, 30, 255));

    const float step = m_gridStep * m_zoom;

    float x0 = fmodf(m_panning.x * m_zoom, step);
    if (x0 < 0.0f) x0 += step;
    for (float x = x0; x < canvasSize.x; x += step)
        dl->AddLine(ImVec2(canvasPos.x + x, canvasPos.y),
            ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y),
            IM_COL32(45, 45, 52, 255));

    float y0 = fmodf(m_panning.y * m_zoom, step);
    if (y0 < 0.0f) y0 += step;
    for (float y = y0; y < canvasSize.y; y += step)
        dl->AddLine(ImVec2(canvasPos.x, canvasPos.y + y),
            ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y),
            IM_COL32(45, 45, 52, 255));
}
//void NodeEditorPanel::DrawGrid(ImDrawList* dl, const ImVec2& canvasPos, const ImVec2& canvasSize)
//{
//    dl->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(26, 26, 30, 255));
//
//    for (float x = fmodf(m_panning.x, m_gridStep); x < canvasSize.x; x += m_gridStep)
//        dl->AddLine(ImVec2(canvasPos.x + x, canvasPos.y), ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y), IM_COL32(45, 45, 52, 255));
//    for (float y = fmodf(m_panning.y, m_gridStep); y < canvasSize.y; y += m_gridStep)
//        dl->AddLine(ImVec2(canvasPos.x, canvasPos.y + y), ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y), IM_COL32(45, 45, 52, 255));
//}

void NodeEditorPanel::DrawLinks(ImDrawList* dl, const ImVec2& origin)
{
    const float tangent = 80.0f;

    for (const auto& l : m_links)
    {
        Pin* from = FindPin(l.fromPinId);
        Pin* to = FindPin(l.toPinId);
        if (!from || !to) continue;

        ImVec2 p1 = GetPinScreenPos(*from, origin);
        ImVec2 p2 = GetPinScreenPos(*to, origin);

        ImVec2 c1(p1.x + tangent, p1.y);
        ImVec2 c2(p2.x - tangent, p2.y);

        dl->AddBezierCubic(p1, c1, c2, p2, IM_COL32(180, 180, 220, 255), 3.0f);

        // interaction hit-box for delete (coarse midpoint test)
        ImVec2 mid((p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f);
        ImGui::SetCursorScreenPos(ImVec2(mid.x - 8, mid.y - 8));

        // hit test: distance from mouse to sampled bezier points
        const ImVec2 mp = ImGui::GetIO().MousePos;
        bool hit = false;
        for (int s = 0; s <= 24 && !hit; ++s)
        {
            const float t = s / 24.0f, u = 1.0f - t;
            ImVec2 q(
                u * u * u * p1.x + 3 * u * u * t * c1.x + 3 * u * t * t * c2.x + t * t * t * p2.x,
                u * u * u * p1.y + 3 * u * u * t * c1.y + 3 * u * t * t * c2.y + t * t * t * p2.y);
            const float dx = q.x - mp.x, dy = q.y - mp.y;
            hit = (dx * dx + dy * dy) < 36.0f; // 6px radius
        }

        if (hit && ImGui::IsWindowHovered() &&
            (ImGui::IsMouseClicked(ImGuiMouseButton_Right) ||
                (ImGui::GetIO().KeyAlt && ImGui::IsMouseClicked(ImGuiMouseButton_Left))))
        {
            RemoveLink(l.id);
            break;
        }

    }
}

void NodeEditorPanel::DrawPendingLink(ImDrawList* dl, const ImVec2& origin)
{
    if (!m_pendingFromPin.has_value()) return;

    Pin* from = FindPin(*m_pendingFromPin);
    if (!from) { m_pendingFromPin.reset(); return; }

    ImVec2 p1 = GetPinScreenPos(*from, origin);
    ImVec2 p2 = ImGui::GetIO().MousePos;
    ImVec2 c1(p1.x + 80.0f, p1.y);
    ImVec2 c2(p2.x - 80.0f, p2.y);

    dl->AddBezierCubic(p1, c1, c2, p2, IM_COL32(240, 210, 120, 255), 2.0f);
}

void NodeEditorPanel::RemoveNode(int nodeId)
{
    // collect this node's pin ids
    std::vector<int> pinIds;
    for (const auto& n : m_nodes)
    {
        if (n.id != nodeId) continue;
        for (const auto& p : n.inputs)  pinIds.push_back(p.id);
        for (const auto& p : n.outputs) pinIds.push_back(p.id);
        break;
    }

    auto hasPin = [&](int pid) {
        return std::find(pinIds.begin(), pinIds.end(), pid) != pinIds.end();
    };

    // remove links touching those pins
    m_links.erase(
        std::remove_if(m_links.begin(), m_links.end(),
            [&](const Link& l) { return hasPin(l.fromPinId) || hasPin(l.toPinId); }),
        m_links.end());

    // remove node
    m_nodes.erase(
        std::remove_if(m_nodes.begin(), m_nodes.end(),
            [&](const Node& n) { return n.id == nodeId; }),
        m_nodes.end());

    if (m_selectedNodeId.has_value() && *m_selectedNodeId == nodeId)
        m_selectedNodeId.reset();
}
void NodeEditorPanel::DrawNodeContents(Node& n)
{
	// used to load a base material type, which will then be used to generate the final material output
    //const char* items[] = { "Select Material",  "Brick", "Wood", "Metal", "Plastic"};
        const char* kinds[] = { "Brick", "Metal", "Wood", "Glass", "Plastic" };

    switch (n.type)
    {
    case NodeType::Material:
    {
       // ImGui::SetNextItemWidth(n.size.x - 40.0f);
        ImGui::SetNextItemWidth((n.size.x - 40.0f) * m_zoom);
        ImGui::Combo("##kind", &n.materialKind, kinds, IM_ARRAYSIZE(kinds));
        break;
    }
	
    case NodeType::Noise:
        ImGui::SliderFloat("Scale", &n.f0, 0.01f, 8.0f);
        ImGui::SliderFloat("Bias", &n.f1, -1.0f, 1.0f);
        break;
	
	case NodeType::Color:
        ImGui::SetNextItemWidth((n.size.x - 60.0f) * m_zoom);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::ColorEdit3("Color", n.col);
		break;
	case NodeType::MortarColor:
        ImGui::SetNextItemWidth((n.size.x - 90.0f) * m_zoom);

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::ColorEdit3("Mortar Color", n.col);
		break;
    case NodeType::Blend:
        ImGui::SetNextItemWidth((n.size.x - 60.0f) * m_zoom);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::SliderFloat("Mix", &n.f0, 0.0f, 1.0f);
        break;
	case NodeType::Depth:
        ImGui::SetNextItemWidth((n.size.x - 60.0f) * m_zoom);

        ImGui::Spacing();
        ImGui::Spacing();
		ImGui::SliderFloat("Depth", &n.f0, 0.0f, 1.0f);
		break;
	case NodeType::Size: 
        ImGui::SetNextItemWidth((n.size.x - 60.0f) * m_zoom);   
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::InputInt("Cols", &n.i0, 1, 32);
        ImGui::SetNextItemWidth((n.size.x - 60.0f) * m_zoom);
        ImGui::InputInt("Rows", &n.i1, 1, 32);
		break;
	case NodeType::Random:
        ImGui::SetNextItemWidth((n.size.x - 60.0f) * m_zoom);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::SliderFloat("Variation", &n.f0, 0.0f, 1.0f);
		break;
    case NodeType::HeightToNormal:ImGui::Spacing();
        ImGui::SetNextItemWidth((n.size.x - 60.0f) * m_zoom);

        ImGui::Spacing();
        ImGui::SliderFloat("Strength", &n.f0, 0.1f, 16.0f);
        break;
	
    case NodeType::Roughness:
        ImGui::SetNextItemWidth((n.size.x - 60.0f) * m_zoom);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::SliderFloat("Contrast", &n.f0, 0.1f, 4.0f);
        {
            bool inv = n.i0 != 0;
            if (ImGui::Checkbox("Invert", &inv)) n.i0 = inv ? 1 : 0;
        }
        break;
    case NodeType::AO:
        ImGui::SetNextItemWidth((n.size.x - 60.0f) * m_zoom);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::SliderFloat("AO", &n.f0, 0.1f, 4.0f);
        {
            bool inv = n.i0 != 0;
            if (ImGui::Checkbox("Invert", &inv)) n.i0 = inv ? 1 : 0;
        }
        break;
    case NodeType::Output:
        ImGui::SetNextItemWidth((n.size.x - 80.0f) * m_zoom);
        ImGui::InputText("Name", n.matName, sizeof(n.matName));
        break;
    
	case NodeType::Albedo:
        ImGui::SetNextItemWidth((n.size.x - 60.0f) * m_zoom);

        //ImGui::TextDisabled("Albedo input");
        break;
    case NodeType::Image:
        ImGui::SetNextItemWidth((n.size.x - 60.0f) * m_zoom);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::InputText("Path", n.path, sizeof(n.path));
        ImGui::SameLine();
        if (ImGui::Button("Browse"))
        {
            const std::string picked = FileDialog::OpenTexture();
            if (!picked.empty())
                strncpy_s(n.path, sizeof(n.path), picked.c_str(), _TRUNCATE);
        }
        break;
    }
}

void NodeEditorPanel::DrawNodes(ImDrawList* dl, const ImVec2& origin)
{
    for (auto& n : m_nodes)
    {
        ImVec2 p = CanvasToScreen(n.pos, origin);
        ImVec2 sz(n.size.x * m_zoom, n.size.y * m_zoom);
        ImVec2 p2(p.x + sz.x, p.y + sz.y);

        const float round = 8.0f * m_zoom;
        const float titleH = 24.0f * m_zoom;

        ImFont* font = ImGui::GetFont();
        const float textSize = ImGui::GetFontSize() * m_zoom; // base size * zoom (computed before any scaling)

        dl->AddRectFilled(p, p2, IM_COL32(52, 52, 60, 240), round);
        dl->AddRect(p, p2, IM_COL32(95, 95, 110, 255), round, 0, 1.5f * m_zoom);

        ImVec2 t2(p2.x, p.y + titleH);
        dl->AddRectFilled(p, t2, IM_COL32(70, 70, 85, 255), round, ImDrawFlags_RoundCornersTop);
        dl->AddText(font, textSize, ImVec2(p.x + 8.0f * m_zoom, p.y + 5.0f * m_zoom),
            IM_COL32(235, 235, 240, 255), n.name.c_str());

        ImGui::SetCursorScreenPos(p);
        ImGui::InvisibleButton(("node_drag_" + std::to_string(n.id)).c_str(), ImVec2(sz.x, titleH));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
        {
            n.pos.x += ImGui::GetIO().MouseDelta.x / m_zoom;
            n.pos.y += ImGui::GetIO().MouseDelta.y / m_zoom;
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            m_selectedNodeId = n.id;

        // ---------- node widgets (scaled) ----------
        ImGui::SetCursorScreenPos(ImVec2(p.x + 8.0f * m_zoom, p.y + 30.0f * m_zoom)); // <-- this was missing
        ImGui::BeginGroup();
        ImGui::PushID(n.id);

        const float oldScale = ImGui::GetCurrentWindow()->FontWindowScale;
        ImGui::SetWindowFontScale(oldScale * m_zoom);

        ImGuiStyle& style = ImGui::GetStyle();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x * m_zoom, style.FramePadding.y * m_zoom));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x * m_zoom, style.ItemSpacing.y * m_zoom));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(style.ItemInnerSpacing.x * m_zoom, style.ItemInnerSpacing.y * m_zoom));
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, style.GrabMinSize * m_zoom);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.FrameRounding * m_zoom);

        ImGui::PushItemWidth(sz.x - 90.0f * m_zoom);
        DrawNodeContents(n);
        ImGui::PopItemWidth();

        ImGui::PopStyleVar(5);
        ImGui::SetWindowFontScale(oldScale);   // reset BEFORE drawing pins

        ImGui::PopID();
        ImGui::EndGroup();
        // ---------- end widgets ----------

        const float pinR = 5.0f * m_zoom;
        const float hit = 16.0f * m_zoom;

        for (auto& pin : n.inputs)
        {
            ImVec2 pp = GetPinScreenPos(pin, origin);
            dl->AddCircleFilled(pp, pinR, PinColor(pin.kind, true));

            ImGui::SetCursorScreenPos(ImVec2(pp.x - hit * 0.5f, pp.y - hit * 0.5f));
            ImGui::InvisibleButton(("pin_in_" + std::to_string(pin.id)).c_str(), ImVec2(hit, hit));

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && m_pendingFromPin.has_value())
            {
                TryCreateLink(*m_pendingFromPin, pin.id);
                m_pendingFromPin.reset();
            }

            dl->AddText(font, textSize,
                ImVec2(pp.x + 10.0f * m_zoom, pp.y - 7.0f * m_zoom),
                IM_COL32(210, 210, 220, 255), pin.name.c_str());
        }

        for (auto& pin : n.outputs)
        {
            ImVec2 pp = GetPinScreenPos(pin, origin);
            dl->AddCircleFilled(pp, pinR, PinColor(pin.kind, false));

            ImGui::SetCursorScreenPos(ImVec2(pp.x - hit * 0.5f, pp.y - hit * 0.5f));
            ImGui::InvisibleButton(("pin_out_" + std::to_string(pin.id)).c_str(), ImVec2(hit, hit));

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                m_pendingFromPin = pin.id;

            ImVec2 txt = font->CalcTextSizeA(textSize, FLT_MAX, 0.0f, pin.name.c_str());
            dl->AddText(font, textSize,
                ImVec2(pp.x - 10.0f * m_zoom - txt.x, pp.y - 7.0f * m_zoom),
                IM_COL32(210, 210, 220, 255), pin.name.c_str());
        }
    }
}

//void NodeEditorPanel::DrawNodes(ImDrawList* dl, const ImVec2& origin)
//{
//    for (auto& n : m_nodes)
//    {
//        ImVec2 p = CanvasToScreen(n.pos, origin);
//        ImVec2 sz(n.size.x * m_zoom, n.size.y * m_zoom);
//        ImVec2 p2(p.x + sz.x, p.y + sz.y);
//
//        const float round = 8.0f * m_zoom;
//        const float titleH = 24.0f * m_zoom;
//
//        dl->AddRectFilled(p, p2, IM_COL32(52, 52, 60, 240), round);
//        dl->AddRect(p, p2, IM_COL32(95, 95, 110, 255), round, 0, 1.5f * m_zoom);
//
//
//
//        ImVec2 t2(p2.x, p.y + titleH);
//        dl->AddRectFilled(p, t2, IM_COL32(70, 70, 85, 255), round, ImDrawFlags_RoundCornersTop);
//        ImFont* font = ImGui::GetFont();
//        const float textSize = ImGui::GetFontSize() * m_zoom;
//        dl->AddText(font, textSize, ImVec2(p.x + 8.0f * m_zoom, p.y + 5.0f * m_zoom),
//            IM_COL32(235, 235, 240, 255), n.name.c_str());
//
//        ImGui::SetCursorScreenPos(p);
//        ImGui::InvisibleButton(("node_drag_" + std::to_string(n.id)).c_str(), ImVec2(sz.x, titleH));
//        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
//        {
//            n.pos.x += ImGui::GetIO().MouseDelta.x / m_zoom;
//            n.pos.y += ImGui::GetIO().MouseDelta.y / m_zoom;
//        }
//
//        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
//            m_selectedNodeId = n.id;
//        //##############################
//
//        ImGui::BeginGroup();
//        ImGui::PushID(n.id);
//
//        // scale font + style for widgets
//        const float oldScale = ImGui::GetCurrentWindow()->FontWindowScale;
//        ImGui::SetWindowFontScale(oldScale * m_zoom);
//
//        ImGuiStyle& style = ImGui::GetStyle();
//        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x * m_zoom, style.FramePadding.y * m_zoom));
//        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x * m_zoom, style.ItemSpacing.y * m_zoom));
//        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(style.ItemInnerSpacing.x * m_zoom, style.ItemInnerSpacing.y * m_zoom));
//        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, style.GrabMinSize * m_zoom);
//        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.FrameRounding * m_zoom);
//
//        ImGui::PushItemWidth(sz.x - 90.0f * m_zoom);
//        DrawNodeContents(n);
//        ImGui::PopItemWidth();
//
//        /*ImGui::PopStyleVar(5);
//        ImGui::SetWindowFontScale(oldScale);
//
//        ImGui::PopID();
//        ImGui::EndGroup();*/
//
//        /*ImGui::SetCursorScreenPos(ImVec2(p.x + 8.0f * m_zoom, p.y + 30.0f * m_zoom));
//        ImGui::BeginGroup();
//        ImGui::PushID(n.id);
//        ImGui::PushItemWidth(sz.x - 90.0f * m_zoom);
//        DrawNodeContents(n);
//        ImGui::PopItemWidth();
//        ImGui::PopID();
//        ImGui::EndGroup();*/
//
//        const float pinR = 5.0f * m_zoom;
//        const float hit = 16.0f * m_zoom;
//
//        for (auto& pin : n.inputs)
//        {
//            ImVec2 pp = GetPinScreenPos(pin, origin);
//            dl->AddCircleFilled(pp, pinR, PinColor(pin.kind, true));
//
//            ImGui::SetCursorScreenPos(ImVec2(pp.x - hit * 0.5f, pp.y - hit * 0.5f));
//            ImGui::InvisibleButton(("pin_in_" + std::to_string(pin.id)).c_str(), ImVec2(hit, hit));
//
//            if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && m_pendingFromPin.has_value())
//            {
//                TryCreateLink(*m_pendingFromPin, pin.id);
//                m_pendingFromPin.reset();
//            }
//
//           /* dl->AddText(ImVec2(pp.x + 10.0f * m_zoom, pp.y - 7.0f * m_zoom),
//                IM_COL32(210, 210, 220, 255), pin.name.c_str());*/
//            dl->AddText(font, textSize,
//                ImVec2(pp.x + 10.0f * m_zoom, pp.y - 7.0f * m_zoom),
//                IM_COL32(210, 210, 220, 255), pin.name.c_str());
//
//        }
//
//        for (auto& pin : n.outputs)
//        {
//            ImVec2 pp = GetPinScreenPos(pin, origin);
//            dl->AddCircleFilled(pp, pinR, PinColor(pin.kind, false));
//
//            ImGui::SetCursorScreenPos(ImVec2(pp.x - hit * 0.5f, pp.y - hit * 0.5f));
//            ImGui::InvisibleButton(("pin_out_" + std::to_string(pin.id)).c_str(), ImVec2(hit, hit));
//
//            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
//                m_pendingFromPin = pin.id;
//
//            ImVec2 txt = ImGui::CalcTextSize(pin.name.c_str());
//            dl->AddText(font, textSize,
//                ImVec2(pp.x - 10.0f * m_zoom - txt.x, pp.y - 7.0f * m_zoom),
//                IM_COL32(210, 210, 220, 255), pin.name.c_str());
//        }
//
//        ImGui::PopStyleVar(5);
//        ImGui::SetWindowFontScale(oldScale);
//
//        ImGui::PopID();
//        ImGui::EndGroup();
//    }
//
//
//}


void NodeEditorPanel::Draw(TextureDocument& doc)
{
    EnsureBootstrapped();

    ImGui::Begin("Node Editor");

    // top bar
    
    if (ImGui::Button("Add Output"))
        NewNode(NodeType::Output, ImVec2(80.0f - m_panning.x, 320.0f - m_panning.y));
    ImGui::SameLine();
    if (ImGui::Button("Evaluate"))
        Evaluate(doc);
    
    
   
    ImGui::Separator();

    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    if (canvasSize.x < 50.0f) canvasSize.x = 50.0f;
    if (canvasSize.y < 50.0f) canvasSize.y = 50.0f;


    ImGui::SetNextItemAllowOverlap();
    ImGui::InvisibleButton("node_canvas", canvasSize,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);

    

    const bool canvasHovered = ImGui::IsItemHovered();
    const bool canvasActive = ImGui::IsItemActive();

    // --------------------------------------------
    // zoom
    //  ----------------------------------------

    ImGuiIO& io = ImGui::GetIO();
    // zoom: only when canvas hovered
    if (canvasHovered && io.MouseWheel != 0.0f)
    {
        const float oldZoom = m_zoom;
        m_zoom = std::clamp(m_zoom * (1.0f + io.MouseWheel * 0.10f), m_minZoom, m_maxZoom);

        ImVec2 mouse = io.MousePos;
        m_panning.x += (mouse.x - canvasPos.x) * (1.0f / m_zoom - 1.0f / oldZoom);
        m_panning.y += (mouse.y - canvasPos.y) * (1.0f / m_zoom - 1.0f / oldZoom);
    }

    // pan with middle mouse drag (divide by zoom)
    if (canvasHovered && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
    {
        ImVec2 d = ImGui::GetIO().MouseDelta;
        m_panning.x += d.x / m_zoom;
        m_panning.y += d.y / m_zoom;
    }

    // IMPORTANT: origin should be canvasPos only
    ImVec2 origin = canvasPos;




    // --------------------------------------------------------------------------------------------------------
	// right-click to open context menu
	// --------------------------------------------------------------------------------------------------------

	if (canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		ImGui::OpenPopup("node_canvas_context");
	}

    

    if (ImGui::BeginPopup("node_canvas_context", ImGuiWindowFlags_None))
    {
        

        if (ImGui::BeginMenu("Add Material")) {
            
            if (ImGui::MenuItem("Add Image")) {
                NewNode(NodeType::Image, ImVec2(80.0f - m_panning.x, 200.0f - m_panning.y));
            }
            if (ImGui::MenuItem("Add Material")) {
                NewNode(NodeType::Material, ImVec2(80.0f - m_panning.x, 200.0f - m_panning.y));
            }
            ImGui::EndMenu();
        }

		if (ImGui::BeginMenu("Add Modifier")) {

            if (ImGui::MenuItem("Add Albedo")) {
                NewNode(NodeType::Albedo, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
            }
			if (ImGui::MenuItem("Add Color")) {
                NewNode(NodeType::Color, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
			}
			if (ImGui::MenuItem("Add Mortar Color")) {
				NewNode(NodeType::MortarColor, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
			}
            if (ImGui::MenuItem("Add Noise")) {
                NewNode(NodeType::Noise, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
            }
            if (ImGui::MenuItem("Add Size")) {
                NewNode(NodeType::Size, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
            }
            if (ImGui::MenuItem("Add Random")) {
                NewNode(NodeType::Random, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
            }
            if (ImGui::MenuItem("Add AO")) {
                NewNode(NodeType::AO, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
            }
            if (ImGui::MenuItem("Add Blend")) {
                NewNode(NodeType::Blend, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
            }
            if (ImGui::MenuItem("Add Depth")) {
                NewNode(NodeType::Depth, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
            }
            if (ImGui::MenuItem("Add Emission")) {
                NewNode(NodeType::Emission, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
            }
            if (ImGui::MenuItem("Add Normal")) {
                NewNode(NodeType::HeightToNormal, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
            }
            if (ImGui::MenuItem("Add Roughness")) {
                NewNode(NodeType::Roughness, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
            }
            if (ImGui::MenuItem("Add Metallic")) {
                NewNode(NodeType::Metallic, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
            }




			ImGui::EndMenu();
		}

        ImGui::EndPopup();
    }

	// ---------------------------------------------------- End context menu ----------------------------------------------------
    

    // pan with middle mouse drag
    
    /*if (canvasHovered && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
    {
        ImVec2 d = ImGui::GetIO().MouseDelta;
        m_panning.x += d.x / m_zoom;
        m_panning.y += d.y / m_zoom;
    }*/
    
      
    
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->PushClipRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), true);

    DrawGrid(dl, canvasPos, canvasSize);

    // editor origin = canvas + pan
    //ImVec2 origin(canvasPos.x + m_panning.x, canvasPos.y + m_panning.y);

    DrawLinks(dl, origin);
    DrawNodes(dl, origin);
    DrawPendingLink(dl, origin);

    // cancel pending link if clicked empty canvas
    if (canvasHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
        m_pendingFromPin.reset();

    dl->PopClipRect();

    if (m_selectedNodeId.has_value() && ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        RemoveNode(*m_selectedNodeId);
    }

    ImGui::End();
}
