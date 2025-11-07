#include "VulkanApplication.hpp"

VulkanApplication::VulkanApplication(const std::string& AppName) :
	context(),

	appName(AppName),

	appInfo {
		.pApplicationName = appName.c_str(),
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14
	},

	createInfo {
		.pApplicationInfo = & appInfo
	},

	instance(context, createInfo)
{
	LOG_DEBUG("Application name is " << appName);
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
