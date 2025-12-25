#pragma once

#include "GlfwContext.hpp"
#include "VulkanInstance.hpp"
#include "VulkanWindow.hpp"
#include "VulkanDevice.hpp"
#include "VulkanLoader.hpp"
#include "Computer.hpp"
#include "Renderer.hpp"
#include "Mesh.hpp"
#include "Satellite.hpp"

class VulkanApplication
{
private:
	const std::string appName;
	GlfwContext glfwContext;
	VulkanInstance vulkanInstance;
	VulkanWindow vulkanWindow;
	VulkanDevice vulkanDevice;
	VulkanLoader vulkanLoader;
	
	SatelliteNetwork satelliteNetwork;
	Mesh m_mesh;

	Computer computer;
	Renderer renderer;

	// Orchestration Sync (Owned by App)
	std::vector<vk::raii::Fence> m_inFlightFences; 
	std::vector<vk::raii::Semaphore> m_computeFinishedSemaphores;
	
	// Mesh and helpers
	void fillMesh();
public:
	VulkanApplication(const std::string&, const std::string&, uint32_t, uint32_t);
	~VulkanApplication();
	int run();
};
