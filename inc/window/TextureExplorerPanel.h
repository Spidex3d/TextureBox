#pragma once
#include <core/TextureDocument.h>

enum class TextureExplorerAction
{
    None,
    OpenFileDialog,
    GenerateNormal,
	SaveAlbedo,
    SaveNormal,
    SaveRoughness,
    ExportMaterialSet
};

class TextureExplorerPanel
{
public:
    TextureExplorerAction DrawTextureExplorer(TextureDocument& doc);

    void Shutdown();

private:
    bool m_isOpen = true;
};