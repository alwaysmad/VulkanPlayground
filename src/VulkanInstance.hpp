// src/VulkanInstance.hpp
#pragma once
#include <vulkan/vulkan_raii.hpp> // For everything Vulkan
#include <vector>
#include <fstream> // for file output

class VulkanInstance {
public:
	VulkanInstance(const std::string& appName, const std::vector<const char*>& requiredExtensions);
	~VulkanInstance();
	
	inline const vk::raii::Instance& getInstance() const { return instance; }
	inline const vk::raii::Context& getContext() const { return context; }
private:
	vk::raii::Context context;
	vk::raii::Instance instance;
	vk::raii::DebugUtilsMessengerEXT debugMessenger;

	static std::ofstream logFile;

	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
			vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
			vk::DebugUtilsMessageTypeFlagsEXT type,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
};
