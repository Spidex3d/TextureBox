#pragma once
#include <imgui/imgui.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>

#include <core/TextureDocument.h>

class NodeEditorPanel
{
public:
    void Draw(TextureDocument& doc);

private:
    enum class NodeType
    {
        Output,
        Image,
        Brick,
		Metal,
		Glass,
		Wood,
		Color,
        Noise,
        Blend,
        HeightToNormal,
        Roughness,
        AO,
		Matallic,
		Emission,
		Transparency,
		Depth,
		Size
    };

    struct Pin
    {
        int id = 0;
        int nodeId = 0;
        bool isInput = true;
        std::string name;
    };

    struct Link
    {
        int id = 0;
        int fromPinId = 0; // output pin
        int toPinId = 0;   // input pin
    };

    struct Node
    {
        int id = 0;
        NodeType type = NodeType::Noise;
        std::string name;
        ImVec2 pos = ImVec2(0, 0);
        ImVec2 size = ImVec2(180, 110);

        char path[260] = {};

        std::vector<Pin> inputs;
        std::vector<Pin> outputs;

        // starter params
        float f0 = 0.5f;
        float f1 = 0.5f;
        int i0 = 4;
        int i1 = 4;

        //brick node color
        float col[3] = { 0.6f, 0.22f, 0.15f };
    };

    

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

  

private:
    // construction
    void EnsureBootstrapped();
    int NewNode(NodeType type, const ImVec2& pos);
    void BuildPinsForNode(Node& n);
    
    void Evaluate(TextureDocument& doc);
    const Node* FindUpstream(int inputPinId) const;

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

    // UI per node
    void DrawNodeContents(Node& n);

   

    // utility
    static const char* NodeTypeName(NodeType t);

};
