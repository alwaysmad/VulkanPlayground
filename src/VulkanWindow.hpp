// src/VulkanWindow.hpp
#pragma once
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN // REQUIRED only for GLFW CreateWindowSurface.
#include <GLFW/glfw3.h>
#include <string>

class VulkanInstance;

class VulkanWindow
{
public:
	VulkanWindow(const VulkanInstance& instance, uint32_t width, uint32_t height, const std::string& name);
	~VulkanWindow();

	inline const vk::raii::SurfaceKHR& getSurface() const { return surface; }
	inline bool shouldClose() const { return glfwWindowShouldClose(window); }
	inline void waitEvents() const { glfwWaitEvents(); }
	inline void pollEvents() const { glfwPollEvents(); }
	vk::Extent2D getExtent() const; 
private:
	GLFWwindow* window;
	vk::raii::SurfaceKHR surface;
};
