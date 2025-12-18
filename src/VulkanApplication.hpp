#pragma once
#include <string>
#include <vector>
#include <cstdlib>

#include "GlfwContext.hpp"
#include "VulkanInstance.hpp"
#include "VulkanWindow.hpp"
#include "VulkanDevice.hpp"
#include "Renderer.hpp"
#include "VulkanLoader.hpp"

class VulkanApplication
{
private:
	const std::string appName;
	GlfwContext glfwContext;
	VulkanInstance vulkanInstance;
	VulkanWindow vulkanWindow;
	VulkanDevice vulkanDevice;
	VulkanLoader vulkanLoader;	
	Renderer renderer;

	// Orchestration Sync (Owned by App)
	std::vector<vk::raii::Fence> m_inFlightFences; 
	// Future: m_computeFinishedSemaphores
	
	// Mesh and helpers
	Mesh m_mesh;
	void fillMesh();
public:
	VulkanApplication(const std::string&, const std::string&, uint32_t, uint32_t);
	~VulkanApplication();
	int run();
};
