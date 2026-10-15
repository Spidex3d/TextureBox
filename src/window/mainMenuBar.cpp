#include "window/mainMenuBar.h"
#include <imgui/imgui.h>
#include <imgui/ImGuiAF.h>
#include <miniBoxLog.h>


MenuAction MainMenuBar::DrawMainMenu()
{
    MenuAction action = MenuAction::None;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            

            if (ImGui::MenuItem("Open Texture"))
            {
                action = MenuAction::OpenTexture;

            }

            if (ImGui::MenuItem("Save Texture"))
            {
                action = MenuAction::SaveTexture;
            }

            ImGui::Separator();

            
            // we can add gltf later if we want to support it.
            ImGui::Separator();

            if (ImGui::MenuItem(
                ICON_FA_SIGN_OUT_ALT " Exit"))
            {
                action = MenuAction::Exit;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo"))
            {
            }

            if (ImGui::MenuItem("Redo"))
            {
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Preferences"))
            {
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Render State"))
        {
            if (ImGui::MenuItem("Render Image"))
            {
            }

            if (ImGui::MenuItem("Render Solid"))
            {
            }

            if (ImGui::MenuItem("Render Wireframe"))
            {
            }

            ImGui::EndMenu();
        }
        // Not sure if we need this menu,we can do this with the right click context menu in viewport.
        if (ImGui::BeginMenu("Add Object"))
        {
            

            ImGui::Separator();

            

            ImGui::EndMenu();
        }

        

        

        // UV menu for UV mapping tools
       
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Help"))
            {
                action = MenuAction::Help;

            }
            if (ImGui::MenuItem("About"))
            {
                action = MenuAction::About;
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Options"))
        {
            static bool enabled = true;
            ImGui::MenuItem("Enabled", "", &enabled);
            // ImGui::BeginChild("child", ImVec2(0, 60), ImGuiChildFlags_Border);
            ImGui::BeginChild("child", ImVec2(0, 60));
            for (int i = 0; i < 10; i++)
                ImGui::Text("Scrolling Text %d", i);
            ImGui::EndChild();
            static float f = 0.5f;
            static int n = 0;
            ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
            ImGui::InputFloat("Input", &f, 0.1f);
            ImGui::InputInt("Input Int", &n);
            ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");

            ImGui::EndMenu();
        }



        ImGui::EndMainMenuBar();
    }

    return action;
}