#pragma once
#include <string>
#include <cstdlib>

#include "GlfwContext.hpp"
#include "VulkanInstance.hpp"
#include "VulkanWindow.hpp"
#include "VulkanDevice.hpp"
#include "VulkanCommand.hpp"
#include "Renderer.hpp"

class VulkanApplication
{
private:
	const std::string appName;
	GlfwContext glfwContext;
	VulkanInstance vulkanInstance;
	VulkanWindow vulkanWindow;
	VulkanDevice vulkanDevice;
	
	// Command Resources (Pools/Buffers)
	VulkanCommand vulkanCommand;
	
	// The Engine Core (Swapchain, Pipeline, Sync, Draw Logic)
	Renderer renderer;

public:
	VulkanApplication(const std::string&, const std::string&, uint32_t, uint32_t);
	~VulkanApplication();
	int run();
};
