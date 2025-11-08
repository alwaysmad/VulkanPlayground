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
	
	vk::InstanceCreateInfo createInfo {
		.pApplicationInfo = &appInfo
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
