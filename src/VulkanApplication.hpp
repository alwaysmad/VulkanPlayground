#pragma once

// Remove vulkan.hpp struct constructors 
// in favor of explicit designated initialization
#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan_raii.hpp> // For everything Vulkan
#include "DebugOutput.hpp"
#include <cstdlib> // For EXIT_SUCCESS and EXIT_FAILURE

class VulkanApplication
{
private:
	/*
	 * Vulkan function loader object.
	 * The first object to create as it finds and loads 
	 * core Vulkan function pointers from system's driver.
	 * Manages function that are not bound to either 
	 * the VkInstance or a VkPhysicalDevice.
	 */
	vk::raii::Context context;
	/*
	 * Required for instance thoughout the program
	 */
	static inline constexpr vk::ApplicationInfo appInfo {
		.pApplicationName   = "SimpleVK",
		.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
		.pEngineName        = "No Engine",
		.engineVersion      = VK_MAKE_VERSION( 1, 0, 0 ),
		.apiVersion         = VK_API_VERSION_1_4
	};
	/*
	 * Struct to pass info to instance constructor
	 */
	static inline constexpr vk::InstanceCreateInfo createInfo {
		.sType            = vk::StructureType::eInstanceCreateInfo,
		.pApplicationInfo = &appInfo
	};
	/*
	 * Vulkan instance
	 * crucial for everything vulkan
	 */
	vk::raii::Instance instance;
public:
	/*
	 * Constructor
	 * handles all initializations
	 */
	VulkanApplication() :
		context(),
		instance(context, createInfo)
	{
		LOG_DEBUG("VulkanApplication created successfully");
	}
	/*
	 * Run the actual programm
	 */
	int run()
	{
		LOG_DEBUG("VulkanApplication instance started run()");
		return EXIT_SUCCESS;
	}
};
