// src/VulkanInstance.cpp
#include "VulkanInstance.hpp"
#include "DebugOutput.hpp"
#include <iostream>
#include <array>

static constexpr const char* engineName = {"SimpleVK"};

static constexpr const char* validationLayersName = { "VK_LAYER_KHRONOS_validation" };

std::ofstream VulkanInstance::logFile;

VulkanInstance::VulkanInstance(const std::string& appName, const GlfwContext& glfw) 
	: context(), instance(nullptr), debugMessenger(nullptr) 
{
	if constexpr (enableValidationLayers)
	{
		const std::string path = "/tmp/" + appName + ".log";
		logFile.open(path);
		LOG_DEBUG("Outputting additional logs to " << path);
	}

	const vk::ApplicationInfo appInfo {
		.pApplicationName = appName.c_str(),
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = engineName,
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion13 // 1.4 required ???
	};

	////////////////////////////////////////////////////////////////////////////////
	// Extensions
	////////////////////////////////////////////////////////////////////////////////
	// Get the required extensions.
	auto requiredExtensions = glfw.getRequiredInstanceExtensions();
	if constexpr (enableValidationLayers)
		requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);

	// Check if the required extensions are supported by the Vulkan implementation.
	const auto extensionProperties = context.enumerateInstanceExtensionProperties();

	if constexpr (enableValidationLayers)
	{
		LOG_DEBUG("Available Vulkan extensions (" << extensionProperties.size() << ") :");
		for ([[maybe_unused]] const auto& i : extensionProperties) { LOG_DEBUG("\t" << i.extensionName); }

		LOG_DEBUG("Required extensions (" << requiredExtensions.size() << ") :");
		for ([[maybe_unused]] const auto& name : requiredExtensions) { LOG_DEBUG("\t" << name); }
	}

	for (const auto& requiredExtension : requiredExtensions)
	{
		if (std::ranges::none_of(
					extensionProperties,
					[requiredExtension](auto const& extensionProperty)
					{ return strcmp(extensionProperty.extensionName, requiredExtension) == 0; }
				))
			{ throw std::runtime_error("Required extension not supported: " + std::string(requiredExtension)); }
	}
	////////////////////////////////////////////////////////////////////////////////
	// Layers
	////////////////////////////////////////////////////////////////////////////////
	// Get the required layers
	std::vector<const char*> requiredLayers;
	if constexpr (enableValidationLayers)
		requiredLayers.push_back(validationLayersName);

	// Check if the required layers are supported by the Vulkan implementation.
	const auto layerProperties = context.enumerateInstanceLayerProperties();

	if constexpr (enableValidationLayers)
	{
		LOG_DEBUG("Available layers (" << layerProperties.size() << ") :");
		for ([[maybe_unused]] const auto& i : layerProperties) { LOG_DEBUG("\t" << i.layerName); }

		LOG_DEBUG("Required layers (" << requiredLayers.size() << ") :");
		for ([[maybe_unused]] const auto& name : requiredLayers) { LOG_DEBUG("\t" << name); }
	}

	for (const auto& requiredLayer : requiredLayers)
	{
		if (std::ranges::none_of(
					layerProperties,
					[requiredLayer](auto const& layerProperty)
					{ return strcmp(layerProperty.layerName, requiredLayer) == 0; }
				))
			{ throw std::runtime_error("Required layer not supported: " + std::string(requiredLayer)); }
	}
	////////////////////////////////////////////////////////////////////////////////
	// Validation layer's debug callback
	////////////////////////////////////////////////////////////////////////////////
	// Set up debug messager callback
	constexpr vk::DebugUtilsMessageSeverityFlagsEXT severityFlags ( 
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | 
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
	);
	constexpr vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags ( 
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
	);
	constexpr vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT {
		.messageSeverity = severityFlags,
		.messageType = messageTypeFlags,
		.pfnUserCallback = &debugCallback
	};
	////////////////////////////////////////////////////////////////////////////////
	// Creating Instance
	////////////////////////////////////////////////////////////////////////////////
	// Create instance (with a debug messanger if enabled)
	const vk::InstanceCreateInfo createInfo {
		.pNext = (enableValidationLayers) ? &debugUtilsMessengerCreateInfoEXT : nullptr,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames = requiredLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
		.ppEnabledExtensionNames = requiredExtensions.data()
	};

	instance = vk::raii::Instance(context, createInfo);
	LOG_DEBUG("Vulkan instance created successfully");

	if constexpr (enableValidationLayers)
	{
		debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
		LOG_DEBUG("Debug callback set up successfully");
	}
	LOG_DEBUG("VulkanInstance initialized");
}

VulkanInstance::~VulkanInstance()
{
	if (logFile.is_open())
		{ logFile.close(); }
	LOG_DEBUG("VulkanInstance destroyed");
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanInstance::debugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
		vk::DebugUtilsMessageTypeFlagsEXT type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void*) 
{
	// Convert 'type' to a string once
	const std::string typeStr = to_string(type);

	// Check severity and direct the output
	if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
	{
		// --- CONSOLE (Warnings and Errors) ---
		std::ostream& outStream = std::cerr;
		const char* stylePrefix = "";
		
		if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
			{ stylePrefix = DBG_COLOR_RED; }
		else if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
			{ stylePrefix = DBG_COLOR_YELLOW; }

		outStream
			<< stylePrefix
			<< DBG_STYLE_BOLD 
			<< typeStr
			<< DBG_COLOR_RESET
			<< stylePrefix
			<< " " << pCallbackData->pMessage
			<< DBG_COLOR_RESET << std::endl;
	}
	else
	{
		// --- LOG FILE (Info and Verbose) ---
		if (logFile.is_open())
			{ logFile << typeStr << " " << pCallbackData->pMessage << std::endl; }
	}

	return vk::False;
}
