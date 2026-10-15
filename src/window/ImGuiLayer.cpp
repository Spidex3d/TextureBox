#include <miniBoxLog.h>
#include <helper.h>
#include <window/ImGuiLayer.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/ImGuiAF.h>

#include <GLFW/glfw3.h>



bool ImGuiLayer::Initialize(GLFWwindow* window)
{

    Helpers helpers; // from Helpers.h, used to get asset paths - fonts, textures etc.
    // ImGui set up
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // enable viewports/docking depending on flag
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    if (m_enableDocking) {
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    }

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 460 core";
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Fonts
    ImFontConfig fontconfig;
    fontconfig.MergeMode = true;
    fontconfig.PixelSnapH = true;
    static const ImWchar ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };


    fontconfig.GlyphOffset = ImVec2(0.0f, 1.0f);
    std::string fontPath = helpers.GetAssetPath(FONT_PATH_MAIN_REL); // comic sans font
    io.Fonts->AddFontFromFileTTF(fontPath.c_str(), FONT_SIZE);

    std::string AFfontPath = helpers.GetAssetPath(FA_SOLID_PATH);
    io.Fonts->AddFontFromFileTTF(AFfontPath.c_str(), FONT_SIZE, &fontconfig, ranges);

    m_initialized = true;
    BOX_LOG_INFO("ImGui initialized successfully");
    return true;
}

void ImGuiLayer::BeginFrame()
{
    // New Frame
    if (!m_initialized)
        return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::SetEnableDocking(bool enabled)
{
    m_enableDocking = enabled;
}

bool ImGuiLayer::GetEnableDocking() const
{
    return m_enableDocking;
}

void ImGuiLayer::MainDockSpace(bool* p_open)
{
    if (m_enableDocking) {
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", p_open, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        ImGui::End();
    }
}

void ImGuiLayer::RenderImGui()
{

    if (!m_initialized)
        return;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backupContext = glfwGetCurrentContext();

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        glfwMakeContextCurrent(backupContext);
    }
}

void ImGuiLayer::ImGuiShutdown()
{
    if (!m_initialized)
        return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_initialized = false;

    BOX_LOG_INFO("ImGui shutdown complete");
}
