#pragma once
//#include <glad/glad.h>
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>	

struct WindowConfig {

	int width = 1280;
	int height = 720;
	const char* title = "Texture Box";
	bool resizable = true;
	bool vsync = true;
};



class Window
{
public:

	explicit Window(const WindowConfig& config);

	GLFWwindow* GetWindow() const { return window; }
	bool ShouldClose() const;

	void Update();
	void Shutdown();

	

	void SetVSync(bool enabled);
	

private:
	GLFWwindow* window = nullptr;

	WindowConfig m_config;

	// Simple refcount so multiple Window instances don't re-init/terminate GLFW
	static int s_glfwRefCount; // static member declaration


};
