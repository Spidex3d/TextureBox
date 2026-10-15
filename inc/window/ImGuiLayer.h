#pragma once

struct GLFWwindow;

class ImGuiLayer
{
public:
    ImGuiLayer() = default;
    ~ImGuiLayer() = default;

    bool Initialize(GLFWwindow* window);
    void BeginFrame();

    // Docking control
    void SetEnableDocking(bool enabled);
    bool GetEnableDocking() const;
    void MainDockSpace(bool* p_open); // docking space

    void RenderImGui(); // finish ImGui frame and render
    void ImGuiShutdown();


private:
    bool m_initialized = false;
    bool m_enableDocking = false;
};


