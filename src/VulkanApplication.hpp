#pragma once

#include <cstdlib>
#include <string>
#include <stdexcept>

#include "GlfwContext.hpp"
#include "VulkanInstance.hpp"
#include "VulkanWindow.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSync.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanPipeline.hpp"
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
	VulkanSwapchain vulkanSwapchain;
	VulkanPipeline vulkanPipeline;
	VulkanSync vulkanSync;
	VulkanCommand vulkanCommand;
	Renderer renderer;

	inline void refreshSwapchain()
	{
		vulkanSwapchain.recreate();
		vulkanSync.refresh(vulkanSwapchain.getImages().size());
	}
public:
	VulkanApplication(const std::string&, const std::string&, uint32_t, uint32_t);
	~VulkanApplication();
	int run();
};
