#include "VulkanApplication.hpp"

VulkanApplication::VulkanApplication(const std::string& AppName) :
	context(),
	appName(AppName),
	instance(nullptr)
{
	LOG_DEBUG("Application name is " << appName);

	vk::ApplicationInfo appInfo {
		.pApplicationName = appName.c_str(),
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14
	};

	// Get the required instance extensions from GLFW.
        uint32_t glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// List glfw required extemtions
	LOG_DEBUG("GLFW required extentions (" << glfwExtensionCount << ") :");
	for (uint32_t i = 0; i < glfwExtensionCount; ++i)
	{
		LOG_DEBUG("\t" << glfwExtensions[i]);
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

	vk::InstanceCreateInfo createInfo {
		.pApplicationInfo = &appInfo,
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
