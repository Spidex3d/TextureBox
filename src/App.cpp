#include "App.h"
#include <miniBoxLog.h>
#include <GLFW/glfw3.h>	
#include <window/window.h>
#include <core\FileDialog.h>
#include <core/NormalMap.h>
#include <fstream>
#include <filesystem>

struct GLFWwindow;

namespace
{
	bool SaveImageWithDialog(const Image& img, const std::string& defaultName, const char* label)
	{
		std::string path = FileDialog::SaveTexture(defaultName);
		if (path.empty()) return false; // user cancelled

		if (path.size() < 4 || path.substr(path.size() - 4) != ".png")
			path += ".png";

		if (!img.SavePNG(path))
		{
			BOX_LOG_ERROR("Failed to save {}: {}", label, path);
			return false;
		}

		BOX_LOG_INFO("{} saved: {} ({} x {})", label, path, img.width, img.height);
		return true;
	}
}


App::App() = default;
App::~App() = default;


bool App::Init()
{

	WindowConfig config;
	config.width = 1280;
	config.height = 720;
	config.title = "TextureBox";
	config.resizable = true;
	config.vsync = true;

	m_window = std::make_unique<Window>(config);

	if (!m_window->GetWindow())
	{
		BOX_LOG_DEBUG("App: Failed to create Window");
		m_window.reset();
		return false;
	}

	m_imgui = std::make_unique<ImGuiLayer>();  // ImGuiLayer for the main window docking and rendering
	m_imgMenu = std::make_unique<MainMenuBar>();                        // Main Menu Bar panel for the main window
	m_imgScene = std::make_unique<TextureViewport>();
	m_imgExplorer = std::make_unique<TextureExplorerPanel>();
	m_preview3D = std::make_unique<Preview3DPanel>();
	m_nodeEditor = std::make_unique<NodeEditorPanel>();
	m_preview3D->Initialize();
	m_imgui->SetEnableDocking(true);

	if (!m_imgui->Initialize(m_window->GetWindow()))
	{
		BOX_LOG_ERROR("Failed to initialize ImGui");

		m_imgui.reset();
		m_imgMenu.reset();
		m_imgScene.reset();
		m_imgExplorer.reset();
		m_preview3D.reset();
		m_window->Shutdown();
		m_window.reset();

		return false;
	}



	m_isRunning = true;

	BOX_LOG_INFO("App initialized successfully");
	return true;
}

int App::Run()
{

	GLFWwindow* nativeWindow = m_window->GetWindow();
	bool dockspaceOpen = true;

	

	while (m_isRunning && !glfwWindowShouldClose(nativeWindow))
	{
		glfwPollEvents();

		glClearColor(0.15f, 0.18f, 0.22f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		m_imgui->BeginFrame();
		m_imgui->MainDockSpace(&dockspaceOpen);

		// Menu
		MenuAction menuAction = m_imgMenu->DrawMainMenu();
		HandleMenuAction(menuAction);

		// Viewport
		TextureViewportAction viewportAction = m_imgScene->DrawTextureViewport(m_doc);
		(void)viewportAction; // for now

		


		// Explorer
		TextureExplorerAction explorerAction = m_imgExplorer->DrawTextureExplorer(m_doc);
		if (explorerAction == TextureExplorerAction::OpenFileDialog)
		{
			std::string path = FileDialog::OpenTexture();
			if (!path.empty())
			{
				if (!m_doc.LoadSource(path))
					BOX_LOG_ERROR("Failed to load texture: {}", path);
				else
					BOX_LOG_INFO("Loaded texture: {}", path);
				
			}
		}

		// #################################### New save Bit ############################
		
		if (explorerAction == TextureExplorerAction::ExportMaterialSet)
		{
			// Ask once for base filename (e.g. "brick01.png")
			std::string basePath = FileDialog::SaveTexture(m_doc.BaseName() + ".png");
			if (!basePath.empty())
			{
				// Ensure .png exists
				if (basePath.size() < 4 || basePath.substr(basePath.size() - 4) != ".png")
					basePath += ".png";

				// Split to: folder + stem
				std::string folder = basePath;
				std::string file = basePath;

				size_t slash = basePath.find_last_of("/\\");
				if (slash != std::string::npos)
				{
					folder = basePath.substr(0, slash + 1);
					file = basePath.substr(slash + 1);
				}
				else
				{
					folder = "";
					file = basePath;
				}

				size_t dot = file.find_last_of('.');
				std::string stem = (dot == std::string::npos) ? file : file.substr(0, dot);

				auto saveOne = [&](bool ready, const Image& img, const std::string& suffix, const char* label)
				{
					if (!ready) return;
					std::string out = folder + stem + "_" + suffix + ".png";
					if (!img.SavePNG(out))
						BOX_LOG_ERROR("Failed to save {}: {}", label, out);
					else
						BOX_LOG_INFO("{} saved: {} ({} x {})", label, out, img.width, img.height);
				};

				saveOne(m_doc.sourceLoaded, m_doc.sourceImage, "albedo", "Albedo");
				saveOne(m_doc.normalReady, m_doc.normalImage, "normal", "Normal");
				saveOne(m_doc.roughnessReady, m_doc.roughnessImage, "roughness", "Roughness");

				// Write .mat sidecar next to textures
				std::string matPath = folder + stem + ".mat";
				std::ofstream mat(matPath, std::ios::out | std::ios::trunc);
				if (!mat)
				{
					BOX_LOG_ERROR("Failed to write material file: {}", matPath);
				}
				else
				{
					mat << "name=" << stem << "\n";
					if (m_doc.sourceLoaded)   mat << "albedo=" << stem << "_albedo.png\n";
					if (m_doc.normalReady)    mat << "normal=" << stem << "_normal.png\n";
					if (m_doc.roughnessReady) mat << "roughness=" << stem << "_roughness.png\n";
					mat << "version=1\n";
					mat.close();

					BOX_LOG_INFO("Material file saved: {}", matPath);
				}
				
			}

			
		}
		

		// ################################# end New Bit ###########################

		// Draw 3D preview
		m_preview3D->Draw(m_doc);
		m_nodeEditor->Draw(m_doc);

		m_imgui->RenderImGui();
		glfwSwapBuffers(nativeWindow);
	}

	Shutdown();
	return 0;
}



void App::HandleMenuAction(MenuAction action)
{
	switch (action)
	{
	case MenuAction::OpenTexture:
	{
		std::string path = FileDialog::OpenTexture();

		if (!path.empty())
		{
			m_doc.sourcePath = path;
			//m_doc.sourceLoaded = m_doc.sourceImage.Load(path);
			if (!m_doc.LoadSource(path))
				BOX_LOG_ERROR("Failed to load texture: {}", path);
			/*if (!m_doc.sourceLoaded)
				BOX_LOG_ERROR("Failed to load texture: {}", path);*/
			else
				BOX_LOG_INFO("Texture loaded from menu: {}", path);
		}
		break;
	}

	case MenuAction::SaveTexture:
	{
		if (!m_doc.normalReady)
		{
			BOX_LOG_ERROR("No normal map generated yet. Click Generate Normal first.");
			break;
		}

		//std::string path = FileDialog::SaveTexture();
		std::string path = FileDialog::SaveTexture(m_doc.SuggestedNormalName());
		if (!path.empty())
		{
			// Make sure the file ends in .png
			if (path.size() < 4 || path.substr(path.size() - 4) != ".png")
				path += ".png";

			if (!m_doc.normalImage.SavePNG(path))
				BOX_LOG_ERROR("Failed to save normal map: {}", path);
			else
				BOX_LOG_INFO("Normal map saved: {} ({} x {})",
					path, m_doc.normalImage.width, m_doc.normalImage.height);
		}
		break;
	}

	

	case MenuAction::Exit:
		glfwSetWindowShouldClose(m_window->GetWindow(), GLFW_TRUE);
		break;
	}
}



void App::Shutdown()
{
	if (m_imgExplorer) m_imgExplorer->Shutdown();
	if (m_imgScene)    m_imgScene->Shutdown();

	if (m_preview3D) { m_preview3D->Shutdown(); m_preview3D.reset(); }

	if (m_imgui)
	{
		m_imgui->ImGuiShutdown();
		m_imgui.reset();
	}

	if (m_window)
	{
		m_window->Shutdown();
		m_window.reset();
	}

	glfwTerminate();
	BOX_LOG_INFO("App shutdown complete");
}