// src/VulkanWindow.cpp
#include "VulkanInstance.hpp"
#include "VulkanWindow.hpp"
#include "DebugOutput.hpp"

VulkanWindow::VulkanWindow(const VulkanInstance& instance, uint32_t width, uint32_t height, const std::string& name) :
	window(nullptr), surface(nullptr), m_lastTime(0.0), m_nbFrames(0)
{
	window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
	if (!window)
	throw std::runtime_error("Failed to create GLFW window");

	VkSurfaceKHR _surface;
	if (glfwCreateWindowSurface(*instance.getInstance(), window, nullptr, &_surface) != VK_SUCCESS)
	throw std::runtime_error("Failed to create window surface");

	surface = vk::raii::SurfaceKHR(instance.getInstance(), _surface);

	// Initialize time
	m_lastTime = glfwGetTime();

	LOG_DEBUG("Window and Surface created");
}

VulkanWindow::~VulkanWindow()
{
	if (window) glfwDestroyWindow(window);
	LOG_DEBUG("Window and Surface destroyed");
}

vk::Extent2D VulkanWindow::getExtent() const
{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	return vk::Extent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}

void VulkanWindow::updateFPS(const std::string& titlePrefix)
{
	double currentTime = glfwGetTime();
	m_nbFrames++;
	if (currentTime - m_lastTime >= 1.0)
	{
		std::string title = titlePrefix + " - " + std::to_string(m_nbFrames) + " FPS";
		glfwSetWindowTitle(window, title.c_str());
		m_nbFrames = 0;
		m_lastTime = currentTime;
	}
}
