#pragma once

#include <cstdlib> // For EXIT_SUCCESS and EXIT_FAILURE
#include <string> // For C++ strings
#include <stdexcept> // For std::runtime_error

#include "GlfwContext.hpp"
#include "VulkanInstance.hpp"
#include "VulkanWindow.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSync.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanRender.hpp"

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
	VulkanRender vulkanRender;
	/*
	 * Helper to refresh swapchain
	 */
	inline void refreshSwapchain()
	{
		vulkanSwapchain.recreate();
		vulkanSync.refresh(vulkanSwapchain.getImages().size());
	}
public:
	/*
	 * Constructor
	 * handles all initializations
	 */
	VulkanApplication(const std::string&, const std::string&, uint32_t, uint32_t);
	/*
	 * Destructor
	 */
	~VulkanApplication();
	/*
	 * Run the actual programm
	 */
	int run();
};
