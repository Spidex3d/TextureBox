#include "window/TextureExplorerPanel.h"
#include <imgui/imgui.h>
#include <cstdint>

TextureExplorerAction TextureExplorerPanel::DrawTextureExplorer(TextureDocument& doc)
{
    TextureExplorerAction action = TextureExplorerAction::None;

    if (!ImGui::Begin("Texture Explorer", &m_isOpen))
    {
        ImGui::End();
        return action;
    }

    if (ImGui::Button("Load Texture..."))
        action = TextureExplorerAction::OpenFileDialog;

    ImGui::Separator();

    if (doc.sourceLoaded && doc.sourceTex.IsValid())
    {
        ImGui::Text("Loaded: %s", doc.sourcePath.c_str());
        ImGui::Text("Size: %d x %d", doc.sourceImage.width, doc.sourceImage.height);

        float maxW = ImGui::GetContentRegionAvail().x;
        float aspect = (float)doc.sourceImage.height / (float)doc.sourceImage.width;
        ImVec2 size(maxW, maxW * aspect);

        ImGui::Image((ImTextureID)(intptr_t)doc.sourceTex.GetID(), size);
    }

    ImGui::Separator();
    ImGui::Text("Normal Map Settings");
    ImGui::SliderFloat("Strength", &doc.normalSettings.strength, 0.1f, 10.0f);
    ImGui::Checkbox("Invert Y", &doc.normalSettings.invertY);
    ImGui::Checkbox("Wrap", &doc.normalSettings.wrap);

    if (ImGui::Button("Generate Normal"))
    {
        if (doc.sourceLoaded)
            action = TextureExplorerAction::GenerateNormal;
    }


    ImGui::End();
    return action;
}

void TextureExplorerPanel::Shutdown() {}