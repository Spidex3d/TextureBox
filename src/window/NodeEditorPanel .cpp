#include "window/NodeEditorPanel.h"
//#include <imgui/imgui_internal.h>
#include <algorithm>
#include <cmath>

const char* NodeEditorPanel::NodeTypeName(NodeType t)
{
    switch (t)
    {
    case NodeType::Output: return "Output";
    case NodeType::Brick: return "Brick";
    case NodeType::Noise: return "Noise";
    case NodeType::Blend: return "Blend";
    case NodeType::HeightToNormal: return "HeightToNormal";
    default: return "Node";
    }
}

void NodeEditorPanel::EnsureBootstrapped()
{
    if (!m_nodes.empty()) return;

    int brick = NewNode(NodeType::Brick, ImVec2(60, 80));
    int h2n = NewNode(NodeType::HeightToNormal, ImVec2(340, 90));
    int out = NewNode(NodeType::Output, ImVec2(640, 110));

    // auto-connect Brick -> HeightToNormal -> Output
    int brickOut = m_nodes[0].outputs[0].id;
    int h2nIn = m_nodes[1].inputs[0].id;
    int h2nOut = m_nodes[1].outputs[0].id;
    int outIn = m_nodes[2].inputs[0].id;

    TryCreateLink(brickOut, h2nIn);
    TryCreateLink(h2nOut, outIn);
}

int NodeEditorPanel::NewNode(NodeType type, const ImVec2& pos)
{
    Node n;
    n.id = m_nextNodeId++;
    n.type = type;
    n.name = NodeTypeName(type);
    n.pos = pos;

    switch (type)
    {
    case NodeType::Output:         n.size = ImVec2(190, 95);  break;
    case NodeType::Brick:          n.size = ImVec2(220, 150); break;
    case NodeType::Noise:          n.size = ImVec2(220, 135); break;
    case NodeType::Blend:          n.size = ImVec2(220, 145); break;
    case NodeType::HeightToNormal: n.size = ImVec2(220, 130); break;
    }

    BuildPinsForNode(n);
    m_nodes.push_back(n);
    return n.id;
}

void NodeEditorPanel::BuildPinsForNode(Node& n)
{
    n.inputs.clear();
    n.outputs.clear();

    auto addIn = [&](const char* name) {
        Pin p; p.id = m_nextPinId++; p.nodeId = n.id; p.isInput = true; p.name = name;
        n.inputs.push_back(p);
    };
    auto addOut = [&](const char* name) {
        Pin p; p.id = m_nextPinId++; p.nodeId = n.id; p.isInput = false; p.name = name;
        n.outputs.push_back(p);
    };

    switch (n.type)
    {
    case NodeType::Output:
        addIn("Surface");
        break;
    case NodeType::Brick:
        addOut("Height");
        break;
    case NodeType::Noise:
        addOut("Value");
        break;
    case NodeType::Blend:
        addIn("A");
        addIn("B");
        addOut("Out");
        break;
    case NodeType::HeightToNormal:
        addIn("Height");
        addOut("Normal");
        break;
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

ImVec2 NodeEditorPanel::GetPinScreenPos(const Pin& pin, const ImVec2& origin) const
{
    // pin layout rule:
    // inputs on left, outputs on right, evenly spaced vertically in each side group
    const Node* node = nullptr;
    for (const auto& n : m_nodes) if (n.id == pin.nodeId) { node = &n; break; }
    if (!node) return origin;

    const float topPad = 30.0f;
    const float step = 22.0f;

    if (pin.isInput)
    {
        int idx = 0;
        for (size_t i = 0; i < node->inputs.size(); ++i) if (node->inputs[i].id == pin.id) { idx = (int)i; break; }
        return ImVec2(origin.x + node->pos.x, origin.y + node->pos.y + topPad + idx * step);
    }
    else
    {
        int idx = 0;
        for (size_t i = 0; i < node->outputs.size(); ++i) if (node->outputs[i].id == pin.id) { idx = (int)i; break; }
        return ImVec2(origin.x + node->pos.x + node->size.x, origin.y + node->pos.y + topPad + idx * step);
    }
}

void NodeEditorPanel::DrawGrid(ImDrawList* dl, const ImVec2& canvasPos, const ImVec2& canvasSize)
{
    dl->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(26, 26, 30, 255));

    for (float x = fmodf(m_panning.x, m_gridStep); x < canvasSize.x; x += m_gridStep)
        dl->AddLine(ImVec2(canvasPos.x + x, canvasPos.y), ImVec2(canvasPos.x + x, canvasPos.y + canvasSize.y), IM_COL32(45, 45, 52, 255));
    for (float y = fmodf(m_panning.y, m_gridStep); y < canvasSize.y; y += m_gridStep)
        dl->AddLine(ImVec2(canvasPos.x, canvasPos.y + y), ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + y), IM_COL32(45, 45, 52, 255));
}

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
        ImGui::InvisibleButton(("link_" + std::to_string(l.id)).c_str(), ImVec2(16, 16));

        if (ImGui::IsItemHovered())
        {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) ||
                (ImGui::GetIO().KeyAlt && ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
            {
                RemoveLink(l.id);
                break;
            }
        }

        /*if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            RemoveLink(l.id);
            break;
        }*/
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
    switch (n.type)
    {
    case NodeType::Brick:
        ImGui::SliderInt("Cols", &n.i0, 1, 32);
        ImGui::SliderInt("Rows", &n.i1, 1, 32);
        break;
    case NodeType::Noise:
        ImGui::SliderFloat("Scale", &n.f0, 0.01f, 8.0f);
        ImGui::SliderFloat("Bias", &n.f1, -1.0f, 1.0f);
        break;
    case NodeType::Blend:
        ImGui::SliderFloat("Mix", &n.f0, 0.0f, 1.0f);
        break;
    case NodeType::HeightToNormal:
        ImGui::SliderFloat("Strength", &n.f0, 0.1f, 16.0f);
        break;
    case NodeType::Output:
        ImGui::TextDisabled("Final material output");
        break;
    }
}

void NodeEditorPanel::DrawNodes(ImDrawList* dl, const ImVec2& origin)
{
    for (auto& n : m_nodes)
    {
        ImVec2 p = ImVec2(origin.x + n.pos.x, origin.y + n.pos.y);
        ImVec2 p2 = ImVec2(p.x + n.size.x, p.y + n.size.y);

        // node body
        dl->AddRectFilled(p, p2, IM_COL32(52, 52, 60, 240), 8.0f);
        dl->AddRect(p, p2, IM_COL32(95, 95, 110, 255), 8.0f, 0, 1.5f);

        // title bar
        ImVec2 t2 = ImVec2(p2.x, p.y + 24.0f);
        dl->AddRectFilled(p, t2, IM_COL32(70, 70, 85, 255), 8.0f, ImDrawFlags_RoundCornersTop);
        dl->AddText(ImVec2(p.x + 8, p.y + 5), IM_COL32(235, 235, 240, 255), n.name.c_str());

        // draggable area
        
        //ImGui::SetNextItemAllowOverlap();

        ImGui::SetCursorScreenPos(p);
        ImGui::InvisibleButton(("node_drag_" + std::to_string(n.id)).c_str(), ImVec2(n.size.x, 24.0f));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
        {
            n.pos.x += ImGui::GetIO().MouseDelta.x;
            n.pos.y += ImGui::GetIO().MouseDelta.y;
        }
        
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            m_selectedNodeId = n.id;
        
        // node UI controls
        ImGui::SetCursorScreenPos(ImVec2(p.x + 8, p.y + 30));
        ImGui::BeginGroup();
        ImGui::PushID(n.id);
        ImGui::PushItemWidth(n.size.x - 90.0f);
        DrawNodeContents(n);
        ImGui::PopItemWidth();
        ImGui::PopID();
        ImGui::EndGroup();
        
        

        // pins + pin interactions
        const float pinR = 5.0f;
        for (auto& pin : n.inputs)
        {
            ImVec2 pp = GetPinScreenPos(pin, origin);
            dl->AddCircleFilled(pp, pinR, IM_COL32(150, 170, 220, 255));

            ImGui::SetCursorScreenPos(ImVec2(pp.x - 8, pp.y - 8));
            ImGui::InvisibleButton(("pin_in_" + std::to_string(pin.id)).c_str(), ImVec2(16, 16));

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && m_pendingFromPin.has_value())
            {
                TryCreateLink(*m_pendingFromPin, pin.id);
                m_pendingFromPin.reset();
            }

            dl->AddText(ImVec2(pp.x + 10, pp.y - 7), IM_COL32(210, 210, 220, 255), pin.name.c_str());
        }

        for (auto& pin : n.outputs)
        {
            ImVec2 pp = GetPinScreenPos(pin, origin);
            dl->AddCircleFilled(pp, pinR, IM_COL32(220, 180, 120, 255));

            ImGui::SetCursorScreenPos(ImVec2(pp.x - 8, pp.y - 8));
            ImGui::InvisibleButton(("pin_out_" + std::to_string(pin.id)).c_str(), ImVec2(16, 16));

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                m_pendingFromPin = pin.id;

            ImVec2 txt = ImGui::CalcTextSize(pin.name.c_str());
            dl->AddText(ImVec2(pp.x - 10 - txt.x, pp.y - 7), IM_COL32(210, 210, 220, 255), pin.name.c_str());
        }
    }
}

void NodeEditorPanel::Draw()
{
    EnsureBootstrapped();

    ImGui::Begin("Node Editor");

    // top bar
    if (ImGui::Button("Add Brick"))
        NewNode(NodeType::Brick, ImVec2(80.0f - m_panning.x, 80.0f - m_panning.y));
    ImGui::SameLine();
    if (ImGui::Button("Add Noise"))
        NewNode(NodeType::Noise, ImVec2(80.0f - m_panning.x, 140.0f - m_panning.y));
    ImGui::SameLine();
    if (ImGui::Button("Add Blend"))
        NewNode(NodeType::Blend, ImVec2(80.0f - m_panning.x, 200.0f - m_panning.y));
    ImGui::SameLine();
    if (ImGui::Button("Add HeightToNormal"))
        NewNode(NodeType::HeightToNormal, ImVec2(80.0f - m_panning.x, 260.0f - m_panning.y));
    ImGui::SameLine();
    if (ImGui::Button("Add Output"))
        NewNode(NodeType::Output, ImVec2(80.0f - m_panning.x, 320.0f - m_panning.y));
    
   
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

    

    // pan with middle mouse drag
    
    if (canvasHovered && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
    {
        ImVec2 d = ImGui::GetIO().MouseDelta;
        m_panning.x += d.x;
        m_panning.y += d.y;
    }

    
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->PushClipRect(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), true);

    DrawGrid(dl, canvasPos, canvasSize);

    // editor origin = canvas + pan
    ImVec2 origin(canvasPos.x + m_panning.x, canvasPos.y + m_panning.y);

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