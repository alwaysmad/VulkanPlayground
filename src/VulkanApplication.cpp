#include "VulkanApplication.hpp"

VulkanApplication::VulkanApplication(const std::string& AppName) :
	context(),
	appName(AppName),
	instance(nullptr)
{
	LOG_DEBUG("Application name is " << appName);

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
	// Get the required instance extensions from GLFW.
        uint32_t glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers)
	{
		extensions.push_back(vk::EXTDebugUtilsExtensionName );
	}

	// List glfw required extemtions
	LOG_DEBUG("Required extentions (" << extensions.size() << ") :");
	for (const auto& i : extensions)
	{
		LOG_DEBUG("\t" << i);
	}

	// Check if the required GLFW extensions are supported by the Vulkan implementation.
        auto extensionProperties = context.enumerateInstanceExtensionProperties();
	// List available extentions
	LOG_DEBUG("Available Vulkan extensions (" << extensionProperties.size() << ") :");
	for (const auto& i : extensionProperties)
	{
		LOG_DEBUG("\t" << i.extensionName);
	}
        for (uint32_t i = 0; i < glfwExtensionCount; ++i)
        {
            if (std::ranges::none_of(extensionProperties,
                                     [glfwExtension = glfwExtensions[i]](auto const& extensionProperty)
                                     { return strcmp(extensionProperty.extensionName, glfwExtension) == 0; }))
            {
                throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
            }
        }
	////////////////////////////////////////////////////////////////////////////////
	// Layers
	////////////////////////////////////////////////////////////////////////////////
	// Get the required layers
	std::vector<char const*> requiredLayers;
	if (enableValidationLayers)
	{
		requiredLayers.assign(validationLayers.begin(), validationLayers.end());
	}
	LOG_DEBUG("Required layers (" << requiredLayers.size() << ") :");
	for (const auto& i : requiredLayers)
	{
		LOG_DEBUG("\t" << i);
	}
	// Check if the required layers are supported by the Vulkan implementation.
	auto layerProperties = context.enumerateInstanceLayerProperties();
	LOG_DEBUG("Available layers (" << layerProperties.size() << ") :");
	for (const auto& i : layerProperties)
	{
		LOG_DEBUG("\t" << i.layerName);
	}
	if (std::ranges::any_of(requiredLayers, [&layerProperties](auto const& requiredLayer) {
		return std::ranges::none_of(layerProperties,
				[requiredLayer](auto const& layerProperty)
				{ return strcmp(layerProperty.layerName, requiredLayer) == 0; });
	}))
	{
		throw std::runtime_error("One or more required layers are not supported!");
	}
	////////////////////////////////////////////////////////////////////////////////
	// Creating Instance
	////////////////////////////////////////////////////////////////////////////////
	vk::InstanceCreateInfo createInfo {
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames = requiredLayers.data(),
		.enabledExtensionCount = glfwExtensionCount,
		.ppEnabledExtensionNames = glfwExtensions
	};
	instance = vk::raii::Instance(context, createInfo);

	LOG_DEBUG("VulkanApplication instance created successfully");
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
