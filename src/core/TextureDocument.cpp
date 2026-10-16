#include "core/TextureDocument.h"
#include <filesystem>


std::string TextureDocument::SuggestedNormalName() const
{
    {
        std::filesystem::path p(sourcePath);
        return p.stem().string() + "_normal.png";
    }
}
