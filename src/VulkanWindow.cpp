// src/VulkanWindow.cpp
#include "VulkanWindow.hpp"
#include "DebugOutput.hpp"

VulkanWindow::VulkanWindow(const vk::raii::Instance& instance, uint32_t width, uint32_t height, const std::string& name) :
	window(nullptr), surface(nullptr)
{
	window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
	if (!window)
		throw std::runtime_error("Failed to create GLFW window");

	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(*instance, window, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface");
        
	surface = vk::raii::SurfaceKHR(instance, surface);
	LOG_DEBUG("Window and Surface created");
}

VulkanWindow::~VulkanWindow()
{
	if (window) glfwDestroyWindow(window);
	LOG_DEBUG("Window and Surface destroyed");
}
