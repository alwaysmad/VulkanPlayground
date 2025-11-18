#include "VulkanApplication.hpp"

static constexpr const char* validationLayersName = { "VK_LAYER_KHRONOS_validation" };
static constexpr std::array deviceExtensionsNames = { 
	vk::KHRSwapchainExtensionName,
	vk::KHRSpirv14ExtensionName,
	vk::KHRSynchronization2ExtensionName,
	vk::KHRCreateRenderpass2ExtensionName
};

std::ofstream VulkanApplication::logFile;

VulkanApplication::VulkanApplication(const std::string& AppName, const std::string& DeviceName) :
	context(),
	appName(AppName),
	deviceName(DeviceName),
	instance(nullptr),
	physicalDevice(nullptr),
	debugMessenger(nullptr)
{
	LOG_DEBUG("Application name is " << appName);

	if constexpr (enableValidationLayers)
	{
		const std::string path = "/tmp/" + appName + ".log";
		logFile.open(path);
		LOG_DEBUG("Outputting additional logs to " << path);
	}

	if (glfwInit() != GLFW_TRUE)
		{ throw std::runtime_error("Failed to initialize GLFW!"); }

	const vk::ApplicationInfo appInfo {
		.pApplicationName = appName.c_str(),
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion13 // 1.4 required ???
	};

	////////////////////////////////////////////////////////////////////////////////
	// Extensions
	////////////////////////////////////////////////////////////////////////////////
	// Get the required extensions.
	const auto requiredExtensions = getRequiredExtensions();

	// Check if the required extensions are supported by the Vulkan implementation.
	const auto extensionProperties = context.enumerateInstanceExtensionProperties();

	LOG_DEBUG("Available Vulkan extensions (" << extensionProperties.size() << ") :");
	for (const auto& i : extensionProperties) { LOG_DEBUG("\t" << i.extensionName); }

	LOG_DEBUG("Required extensions (" << requiredExtensions.size() << ") :");
	for (const auto& name : requiredExtensions) { LOG_DEBUG("\t" << name); }

	for (const auto& requiredExtension : requiredExtensions)
	{
		if (std::ranges::none_of(
					extensionProperties,
					[requiredExtension](auto const& extensionProperty)
					{ return strcmp(extensionProperty.extensionName, requiredExtension) == 0; }
				))
			throw std::runtime_error("Required extension not supported: " + std::string(requiredExtension));
	}

	////////////////////////////////////////////////////////////////////////////////
	// Layers
	////////////////////////////////////////////////////////////////////////////////
	// Get the required layers
	const auto requiredLayers = getRequiredLayers();

	// Check if the required layers are supported by the Vulkan implementation.
	const auto layerProperties = context.enumerateInstanceLayerProperties();

	LOG_DEBUG("Available layers (" << layerProperties.size() << ") :");
	for (const auto& i : layerProperties) { LOG_DEBUG("\t" << i.layerName); }

	LOG_DEBUG("Required layers (" << requiredLayers.size() << ") :");
	for (const auto& name : requiredLayers) { LOG_DEBUG("\t" << name); }

	for (const auto& requiredLayer : requiredLayers)
	{
		if (std::ranges::none_of(
					layerProperties,
					[requiredLayer](auto const& layerProperty)
					{ return strcmp(layerProperty.layerName, requiredLayer) == 0; }
				))
			throw std::runtime_error("Required layer not supported: " + std::string(requiredLayer));
	}

	////////////////////////////////////////////////////////////////////////////////
	// Validation layer's debug callback
	////////////////////////////////////////////////////////////////////////////////
	// Set up debug messager callback
	constexpr vk::DebugUtilsMessageSeverityFlagsEXT severityFlags ( 
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | 
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError );

	constexpr vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags ( 
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation );

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
	////////////////////////////////////////////////////////////////////////////////
	// Selecting a physical device
	////////////////////////////////////////////////////////////////////////////////
	const auto devices = instance.enumeratePhysicalDevices();

	LOG_DEBUG("Available Physical Devices (" << devices.size() << ") :");
	for (const auto& device : devices)
	{
		// Get properties (name, type, API version)
		const auto props = device.getProperties();
		// Get features (geometry shader, tessellation, etc.)
		// const auto features = device.getFeatures();
		LOG_DEBUG("\t" << props.deviceName);
		// If we found the requested name, grab it immediately
		if ( std::string(props.deviceName) == deviceName )
			{ physicalDevice = device; }
	}

	if (*physicalDevice == nullptr) // User asked for a name, but we didn't find it
		{ throw std::runtime_error("Could not find requested device: " + deviceName); }
	else
		{ LOG_DEBUG("Successfully selected requested device: '"
				<< physicalDevice.getProperties().deviceName << "'."); }

	// Check some properties
	const auto props = physicalDevice.getProperties();
	
	if (props.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
	{
		std::cerr << DBG_COLOR_YELLOW
				  << "[WARNING] Selected device '" << deviceName
				  << "' is NOT a discrete GPU."
				  << DBG_COLOR_RESET << std::endl;
	}
	if (props.apiVersion <= vk::ApiVersion13)
		{ throw std::runtime_error("Selected device " + deviceName + " does not support Vulkan 1.3"); }

	const auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

	LOG_DEBUG("Available device extensions (" << availableExtensions.size() << ") :");
	for (const auto& i : availableExtensions) { LOG_DEBUG("\t" << i.extensionName); }

	const auto requiredDeviceExtensions = getRequiredDeviceExtensions();
	LOG_DEBUG("Required device extensions (" << requiredDeviceExtensions.size() << ") :");
	for (const auto& i : requiredDeviceExtensions) { LOG_DEBUG("\t" << i); }

	for (const auto& requiredDeviceExtension: requiredDeviceExtensions)
	{
		if (std::ranges::none_of(
					availableExtensions,
					[requiredDeviceExtension](auto const& extensionProperty)
					{ return strcmp(extensionProperty.extensionName, requiredExtension) == 0; }
				))
			throw std::runtime_error("Required device extension not supported: " + std::string(requiredExtension));
	}
}

VulkanApplication::~VulkanApplication()
{
	LOG_DEBUG("VulkanApplication instance destroyed");
	glfwTerminate();
	if (logFile.is_open())
		{ logFile.close(); }
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

	// Add validation layers if required
	if constexpr (enableValidationLayers)
		requiredLayers.push_back(validationLayersName);

	return requiredLayers;
}

std::vector<const char*> VulkanApplication::getRequiredDeviceExtensions()
{
	std::vector<const char*> deviceExtensions;
	
	deviceExtensions.assign(deviceExtensionsNames.begin(), deviceExtensionsNames.end());
	
	return deviceExtensions;
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanApplication::debugCallback (
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
