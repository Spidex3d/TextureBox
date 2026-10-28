#include "core/TextureDocument.h"
#include <filesystem>

std::string TextureDocument::BaseName() const
{
    if (!materialName.empty()) return materialName;

    std::filesystem::path p(sourcePath);
    std::string s = p.stem().string();
    return s.empty() ? "material" : s;
}

std::string TextureDocument::SuggestedAlbedoName() const { return BaseName() + "_albedo.png"; }
std::string TextureDocument::SuggestedNormalName() const { return BaseName() + "_normal.png"; }
std::string TextureDocument::SuggestedRoughnessName() const { return BaseName() + "_roughness.png"; }
std::string TextureDocument::SuggestedAOName() const { return BaseName() + "_ao.png"; }
std::string TextureDocument::SuggestedDisplacementName() const { return BaseName() + "_displacement.png"; }
std::string TextureDocument::SuggestedSpecularName() const { return BaseName() + "_specular.png"; }
