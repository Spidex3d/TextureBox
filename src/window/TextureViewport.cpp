#include "window/TextureViewport.h"
#include <imgui/imgui.h>
#include <algorithm>
#include <cstdint>

TextureViewportAction TextureViewport::DrawTextureViewport(const TextureDocument& doc)
{
    TextureViewportAction action = TextureViewportAction::None;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
    ImGui::Begin("Main Texture Viewport");

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::Text("Viewport area: %.0f x %.0f", avail.x, avail.y);

    /*const GLTexture* tex = doc.normalReady ? &doc.normalTex
        : (doc.sourceLoaded ? &doc.sourceTex : nullptr);*/

    const GLTexture* tex = nullptr;

    if (doc.normalReady) tex = &doc.normalTex;
    else if (doc.sourceLoaded) tex = &doc.sourceTex;


    if (tex && tex->IsValid())
    {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float scale = std::min(avail.x / tex->Width(), avail.y / tex->Height());
        ImVec2 size(tex->Width() * scale, tex->Height() * scale);

        // centre it
        ImVec2 cur = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(cur.x + (avail.x - size.x) * 0.5f,
            cur.y + (avail.y - size.y) * 0.5f));
        ImGui::Image((ImTextureID)(intptr_t)tex->GetID(), size);
    }
    else
    {
        ImGui::TextDisabled("Load a texture from the Texture Explorer.");
    }

    


    ImGui::End();
    ImGui::PopStyleVar();
    return action;
}

void TextureViewport::Shutdown() {}
