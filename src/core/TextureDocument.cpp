#include "core/TextureDocument.h"
#include <filesystem>


//std::string TextureDocument::SuggestedAlbedoName() const
//{
//    std::filesystem::path p(sourcePath);
//    return p.stem().string() + "_albedo.png";
//}
//
//std::string TextureDocument::SuggestedNormalName() const
//{
//    {
//        std::filesystem::path p(sourcePath);
//        return p.stem().string() + "_normal.png";
//    }
//}
//
//std::string TextureDocument::SuggestedRoughnessName() const
//{
//	std::filesystem::path p(sourcePath);
//	return p.stem().string() + "_roughness.png";
//}
//    
//
//std::string TextureDocument::SuggestedAOName() const
//{
//    std::filesystem::path p(sourcePath);
//    return p.stem().string() + "a_ao.png";
//}
//
//std::string TextureDocument::SuggestedDisplacementName() const
//{
//    std::filesystem::path p(sourcePath);
//    return p.stem().string() + "a_displacement.png";
//}

std::string TextureDocument::SuggestedSpecularName() const
{
    std::filesystem::path p(sourcePath);
    return p.stem().string() + "a_specular.png";
}

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


