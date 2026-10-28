#pragma once
#include <core/TextureDocument.h>

enum class TextureViewportAction
{
    None
};

enum class ViewMapMode
{
    Auto = 0,
    Albedo,
    Normal,
    Roughness,
    AO,
    Displacement,
    Specular,
    Emission
};

class TextureViewport
{
public:
    TextureViewportAction DrawTextureViewport(const TextureDocument& doc);
    void Shutdown();

private:
    

    ViewMapMode m_viewMode = ViewMapMode::Albedo;
    bool        m_showLarge = false;
    ViewMapMode m_largeMode = ViewMapMode::Albedo;
};