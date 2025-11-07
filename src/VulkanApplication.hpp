#pragma once

// Remove vulkan.hpp struct constructors 
// in favor of explicit designated initialization
#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan_raii.hpp> // For everything Vulkan
#include <cstdlib> // For EXIT_SUCCESS and EXIT_FAILURE
#include <string>
#include "DebugOutput.hpp"

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
	 * Vulkan instance
	 * crucial for everything vulkan
	 */
	vk::raii::Instance instance;
public:
	/*
	 * Constructor
	 * handles all initializations
	 */
	explicit VulkanApplication(const std::string&);
	/*
	 * Destructor
	 */
	~VulkanApplication();
	/*
	 * Run the actual programm
	 */
	int run();
};
