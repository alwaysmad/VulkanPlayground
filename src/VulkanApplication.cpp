#include "VulkanApplication.hpp"

inline static const char* findMissingItem (
		const std::vector<const char*>& required,
		const std::vector<const char*>& available )
{
	for (const char* requiredName : required)
	{
		const bool found = std::ranges::any_of( available,
			[requiredName] (const char* availableName)
				{ return strcmp(requiredName, availableName) == 0;}
		);

		// return missing name
		if (!found) return requiredName;
	}
	return nullptr; // All items were found
}

VulkanApplication::VulkanApplication(const std::string& AppName) :
	context(),
	appName(AppName),
	instance(nullptr),
	debugMessenger(nullptr)
{
	LOG_DEBUG("Application name is " << appName);

	// Create temporary AppInfo struct
	const vk::ApplicationInfo appInfo {
		.pApplicationName = appName.c_str(),
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14
	};

	////////////////////////////////////////////////////////////////////////////////
	// Extensions
	////////////////////////////////////////////////////////////////////////////////
	const std::vector<const char*> requiredExtensions = getRequiredExtensions();
	std::vector<const char*> availableExtensionNames;

	auto availableExtensionProps = context.enumerateInstanceExtensionProperties();

	availableExtensionNames.reserve(availableExtensionProps.size());
	for (const auto& prop : availableExtensionProps)
		availableExtensionNames.push_back(prop.extensionName);

	LOG_DEBUG("Available Vulkan extensions (" << availableExtensionNames.size() << ") :");
	for (const auto& name : availableExtensionNames) { LOG_DEBUG("\t" << name); }
	
	LOG_DEBUG("Required extensions (" << requiredExtensions.size() << ") :");
	for (const auto& name : requiredExtensions) { LOG_DEBUG("\t" << name); }

	if (const char* missingExt = findMissingItem(requiredExtensions, availableExtensionNames))
		{ throw std::runtime_error("Required extension not supported: " + std::string(missingExt)); }

	////////////////////////////////////////////////////////////////////////////////
	// Layers
	////////////////////////////////////////////////////////////////////////////////
	const std::vector<const char*> requiredLayers = getRequiredLayers();
	std::vector<const char*> availableLayerNames;

	auto availableLayerProps = context.enumerateInstanceLayerProperties();

	availableLayerNames.reserve(availableLayerProps.size());
	for (const auto& prop : availableLayerProps)
		availableLayerNames.push_back(prop.layerName);
	
	LOG_DEBUG("Available layers (" << availableLayerNames.size() << ") :");
	for (const auto& name : availableLayerNames) { LOG_DEBUG("\t" << name); }

	LOG_DEBUG("Required layers (" << requiredLayers.size() << ") :");
	for (const auto& name : requiredLayers) { LOG_DEBUG("\t" << name); }

	if (const char* missingLayer = findMissingItem(requiredLayers, availableLayerNames))
		{ throw std::runtime_error("Required layer not supported: " + std::string(missingLayer)); }

	////////////////////////////////////////////////////////////////////////////////
	// Creating Instance
	////////////////////////////////////////////////////////////////////////////////
	const vk::InstanceCreateInfo createInfo {
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames = requiredLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
		.ppEnabledExtensionNames = requiredExtensions.data()
	};
	instance = vk::raii::Instance(context, createInfo);

	LOG_DEBUG("VulkanApplication instance created successfully");

	////////////////////////////////////////////////////////////////////////////////
	// Validation layer's debug callback
	////////////////////////////////////////////////////////////////////////////////
	if constexpr (enableValidationLayers)
	{
		const vk::DebugUtilsMessageSeverityFlagsEXT severityFlags ( 
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | 
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eError );

		const vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags ( 
				vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
				vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
				vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation );

		const vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
		    .messageSeverity = severityFlags,
		    .messageType = messageTypeFlags,
		    .pfnUserCallback = &debugCallback
		    };
		
		debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);

		LOG_DEBUG("Debug callback setup successfully");
	}
}

VulkanApplication::~VulkanApplication()
{
	LOG_DEBUG("VulkanApplication instance destroyed");
}

int VulkanApplication::run()
{
	LOG_DEBUG("VulkanApplication instance started run()");
	return EXIT_SUCCESS;
}

std::vector<const char*> VulkanApplication::getRequiredExtensions()
{
	// Add GLFW required extentions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	// Add Debug extension is validation layers are required
	if constexpr (enableValidationLayers)
		extensions.push_back(vk::EXTDebugUtilsExtensionName);

	return extensions;
}

std::vector<const char*> VulkanApplication::getRequiredLayers()
{
	std::vector<const char*> requiredLayers;
	// Add validation layers is required
	if constexpr (enableValidationLayers)
		requiredLayers.assign(validationLayers.begin(), validationLayers.end());
	return requiredLayers;
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanApplication::debugCallback (
		vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
		vk::DebugUtilsMessageTypeFlagsEXT type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void*)
{
	std::cout << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

    return vk::False;
}
