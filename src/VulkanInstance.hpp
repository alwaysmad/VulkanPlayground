// src/VulkanInstance.hpp
#pragma once
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp> // For everything Vulkan
#include "GlfwContext.hpp"
#include <vector>
#include <fstream> // for file output

class VulkanInstance {
public:
	VulkanInstance(const std::string& appName, const GlfwContext& glfw);
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
