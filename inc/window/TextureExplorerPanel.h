#pragma once
#include <core/TextureDocument.h>

enum class TextureExplorerAction
{
    None,
    OpenFileDialog,
    GenerateNormal,
    SaveNormal
};

class TextureExplorerPanel
{
public:
    TextureExplorerAction DrawTextureExplorer(TextureDocument& doc);

    void Shutdown();

private:
    bool m_isOpen = true;
};