#pragma once
#include <imgui/imgui.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>

#include <core/TextureDocument.h>

#include "core/NodeType.h"

class NodeEditorPanel
{
public:
    void Draw(TextureDocument& doc);



private:
    // graph data
    std::vector<Node> m_nodes;
    std::vector<Link> m_links;

    std::optional<int> m_selectedNodeId;
    // editor state
    ImVec2 m_panning = ImVec2(0, 0);
    float m_gridStep = 32.0f;

    int m_nextNodeId = 1;
    int m_nextPinId = 1;
    int m_nextLinkId = 1;

    std::optional<int> m_pendingFromPin; // output pin being dragged

    

    // zoom
    float m_zoom = 1.0f;
    float m_minZoom = 0.60f;
    float m_maxZoom = 1.80f;
  
    ImVec2 CanvasToScreen(const ImVec2& p, const ImVec2& origin) const;
    ImVec2 ScreenToCanvas(const ImVec2& p, const ImVec2& origin) const;

private:
    // construction
    void EnsureBootstrapped();
    int NewNode(NodeType type, const ImVec2& pos);
    void BuildPinsForNode(Node& n);
    
    void Evaluate(TextureDocument& doc);
    const Node* FindUpstream(int inputPinId) const;
    const Node* ResolveAlbedoSource(const Node* n) const;
    // drawing
    void DrawGrid(ImDrawList* dl, const ImVec2& canvasPos, const ImVec2& canvasSize);
    void DrawLinks(ImDrawList* dl, const ImVec2& origin);
    void DrawNodes(ImDrawList* dl, const ImVec2& origin);
    void DrawPendingLink(ImDrawList* dl, const ImVec2& origin);
    void RemoveNode(int nodeId);
    // helpers
    Node* FindNode(int nodeId);
    Pin* FindPin(int pinId);
    ImVec2 GetPinScreenPos(const Pin& pin, const ImVec2& origin) const;
    bool IsPinConnectedAsInput(int pinId) const;
    bool CanCreateLink(int fromPinId, int toPinId) const;
    void TryCreateLink(int fromPinId, int toPinId);
    void RemoveLink(int linkId);
    static ImU32 PinColor(PinKind k, bool isInput);

    // UI per node
    void DrawNodeContents(Node& n);

    // utility
    static const char* NodeTypeName(NodeType t);
   
};
