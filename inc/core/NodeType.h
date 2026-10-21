#pragma once
#include <imgui/imgui.h>
#include <vector>
#include <string>


enum class NodeType
{
    // Special
    Output,
    Albedo,

    // Inputs / Sources
    Image,
    Color,
    MortarColor,
    Random,

    // Material Generators
    Material,

    // Modifiers
    Noise,
    Blend,
    Depth,
    Size,
    Bevel,
    EdgeSoften,
    FacePitting,
    Shininess,
    //wood
    Planks,
    Rings,
    GrainScale,
    SeamWidth,
    Warp,
    // Rock
	Cracks,
	Scale,
	Detail,
    // Metal 
	Rust,
    Scratches,

    

	

    // Map Generators / Converters
    HeightToNormal,
    Roughness,
    AO,
    Metallic,
    Emission,
    Transparency
};

enum class MaterialKind
{
    Brick = 0,
    Metal,
    Wood,
    Glass,
    Plastic,
    Rock
};
enum class PinKind
{
    Material,   // chain data (Material -> Size -> Color -> Random -> Output.Albedo)
    Image,      // bitmap/map data
    Scalar,     // single float/int (future)
    Color,      // RGB (future)
    Albedo,
    Surface
};


struct Pin
{
    int id = 0;
    int nodeId = 0;
    bool isInput = true;
    std::string name;
    PinKind kind = PinKind::Material;
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

    char matName[64] = "Brick";

    std::vector<Pin> inputs;
    std::vector<Pin> outputs;

    // starter params
    float f0 = 0.5f;
    float f1 = 0.5f;
    int i0 = 4;
    int i1 = 4;

    //brick node color
    float col[3] = { 0.6f, 0.22f, 0.15f };

    int materialKind = (int)MaterialKind::Brick; // default
};
