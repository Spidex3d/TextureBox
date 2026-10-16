#pragma once
#include <memory>
#include <window/ImGuiLayer.h>
#include <window/mainMenuBar.h>
#include <window/TextureViewport.h>
#include <window/TextureExplorerPanel.h>
#include <window/Preview3DPanel.h>
#include <core/TextureDocument.h>



class Window;

class App
{

public:
	App();
	~App();

	bool Init(); // Initialize the window and ImGui context
	int Run();        // set the main loop
	void Shutdown();

private:
	TextureDocument m_doc;
	void HandleMenuAction(MenuAction action);

	std::unique_ptr<Window> m_window;			// Window instance for managing the application window

	

	// imGuiLayer m_imguiLayer;	// ImGuiLayer instance for managing ImGui context and rendering
	std::unique_ptr<ImGuiLayer> m_imgui;
	std::unique_ptr<MainMenuBar> m_imgMenu;			// menu bar panel for ImGui
	std::unique_ptr<TextureViewport> m_imgScene; // Texture viewport panel for ImGui
	std::unique_ptr<TextureExplorerPanel> m_imgExplorer; // Texture explorer panel for ImGui
	std::unique_ptr<Preview3DPanel> m_preview3D;

	bool m_isRunning = false;	// Flag to indicate if the application is running



};
