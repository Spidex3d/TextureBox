#include "window/TextureViewport.h"
#include <imgui/imgui.h>
#include <algorithm>
#include <cstdint>

static const GLTexture* GetMapTexture(const TextureDocument& doc, ViewMapMode m)
{
    switch (m)
    {
    case ViewMapMode::Albedo:
        return (doc.sourceLoaded && doc.sourceTex.IsValid()) ? &doc.sourceTex : nullptr;
    case ViewMapMode::Normal:
        return (doc.normalReady && doc.normalTex.IsValid()) ? &doc.normalTex : nullptr;
    case ViewMapMode::Roughness:
        return (doc.roughnessReady && doc.roughnessTex.IsValid()) ? &doc.roughnessTex : nullptr;
	case ViewMapMode::AO:
		return (doc.aoReady && doc.aoTex.IsValid()) ? &doc.aoTex : nullptr;
    default:
        return nullptr; // AO, Displacement, Specular: not implemented yet
    }
}

static void DrawFitted(const GLTexture* tex)
{
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (avail.x < 1 || avail.y < 1) return;
    float scale = std::min(avail.x / tex->Width(), avail.y / tex->Height());
    ImVec2 size(tex->Width() * scale, tex->Height() * scale);
    ImVec2 cur = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(cur.x + (avail.x - size.x) * 0.5f,
        cur.y + (avail.y - size.y) * 0.5f));
    ImGui::Image((ImTextureID)(intptr_t)tex->GetID(), size);
}

TextureViewportAction TextureViewport::DrawTextureViewport(const TextureDocument& doc)
{
    TextureViewportAction action = TextureViewportAction::None;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
    ImGui::Begin("Main Texture Viewport");

    struct Entry { const char* label; ViewMapMode mode; };
    const Entry entries[] = {
        { "Albedo",       ViewMapMode::Albedo },
        { "Normal",       ViewMapMode::Normal },
        { "Roughness",    ViewMapMode::Roughness },
        { "AO",           ViewMapMode::AO },
        { "Displacement", ViewMapMode::Displacement },
        { "Specular",     ViewMapMode::Specular },
    };

    // ---- thumbnail strip ----
    const ImVec2 thumb(72, 72);
    for (const Entry& e : entries)
    {
        const GLTexture* t = GetMapTexture(doc, e.mode);
        const bool selected = (m_viewMode == e.mode);

        ImGui::BeginGroup();
        ImGui::PushID((int)e.mode);

        
        if (selected)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.7f, 0.2f, 1.0f));

        bool clicked;
        if (t)
            clicked = ImGui::ImageButton("##thumb", (ImTextureID)(intptr_t)t->GetID(), thumb);
        else
            clicked = ImGui::Button("n/a", ImVec2(thumb.x + 8, thumb.y + 8));

        if (selected) ImGui::PopStyleColor();

        if (clicked) m_viewMode = e.mode;

        if (ImGui::IsItemHovered() && t)
        {
            ImGui::SetTooltip("%s\nDouble-click to open large preview", e.label);

            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                m_largeMode = e.mode;
                m_showLarge = true;
            }

        }

        ImGui::TextUnformatted(e.label);
        ImGui::PopID();
        ImGui::EndGroup();
        ImGui::SameLine();
    }
    ImGui::NewLine();
    ImGui::Separator();

    // ---- selected map, fitted to the panel ----
    const GLTexture* tex = GetMapTexture(doc, m_viewMode);
    if (tex)
        DrawFitted(tex);
    else
        ImGui::TextDisabled("Selected map is not available yet.");

    ImGui::End();
    ImGui::PopStyleVar();

    // ---- large preview window ----
    if (m_showLarge)
    {
        ImGui::SetNextWindowSize(ImVec2(700, 700), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Map Preview", &m_showLarge))
        {
            const GLTexture* big = GetMapTexture(doc, m_largeMode);
            if (big) DrawFitted(big);
            else ImGui::TextDisabled("Map not available.");
        }
        ImGui::End();
    }

    return action;
}

void TextureViewport::Shutdown() {}
