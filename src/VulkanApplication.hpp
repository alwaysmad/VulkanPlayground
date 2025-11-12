#pragma once

#include <cstdlib> // For EXIT_SUCCESS and EXIT_FAILURE
#include <string> // For C++ strings
#include <algorithm> // For std::ranges
#include <vector>
#include <array>

// Remove vulkan.hpp struct constructors 
// in favor of explicit designated initialization
#define VULKAN_HPP_NO_CONSTRUCTORS

#include <vulkan/vulkan_raii.hpp> // For everything Vulkan

#define GLFW_INCLUDE_VULKAN // REQUIRED only for GLFW CreateWindowSurface.
#include <GLFW/glfw3.h>

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
	 * Application name
	 */
	const std::string appName;
#ifdef NDEBUG
	static constexpr bool enableValidationLayers = false;
#else
	static constexpr bool enableValidationLayers = true;
	static constexpr std::array validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};
#endif
	/*
	 * Vulkan instance
	 * crucial for everything vulkan
	 */

	vk::raii::Instance instance;
	/*
	 * Functions that returns required extentions
	 * and required layers
	 */
	static std::vector<const char*> getRequiredExtensions();
	static std::vector<const char*> getRequiredLayers();
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
