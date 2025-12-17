// src/VulkanWindow.hpp
#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#include <string>

class VulkanInstance;

class VulkanWindow
{
public:
	VulkanWindow(const VulkanInstance& instance, uint32_t width, uint32_t height, const std::string& name);
	~VulkanWindow();

	// New Helper: Handles time checking and window title updating
	void updateFPS(const std::string& titlePrefix);

	inline const vk::raii::SurfaceKHR& getSurface() const { return surface; }
	inline bool shouldClose() const { return glfwWindowShouldClose(window); }
	inline void waitEvents() const { glfwWaitEvents(); }
	inline void pollEvents() const { glfwPollEvents(); }
	vk::Extent2D getExtent() const; 
	inline void setWindowTitle(const std::string& title) { glfwSetWindowTitle(window, title.c_str()); }
	inline double getTime() const { return glfwGetTime(); }

private:
	GLFWwindow* window;
	vk::raii::SurfaceKHR surface;

	// FPS State
	double m_lastTime = 0.0;
	uint32_t m_nbFrames = 0;
};
