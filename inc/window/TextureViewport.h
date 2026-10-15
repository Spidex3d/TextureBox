#pragma once
#include <core/TextureDocument.h>

enum class TextureViewportAction
{
    None
};

class TextureViewport
{
public:
    TextureViewportAction DrawTextureViewport(const TextureDocument& doc);
    void Shutdown();
};