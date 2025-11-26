#include "VulkanApplication.hpp"
#include "DebugOutput.hpp"

VulkanApplication::VulkanApplication(const std::string& AppName, const std::string& DeviceName, uint32_t w, uint32_t h) :
	appName(AppName),
	glfwContext(),
	vulkanInstance(appName, glfwContext),
	vulkanWindow(vulkanInstance.getInstance(), w, h, appName),
	vulkanDevice(vulkanInstance.getInstance(), vulkanWindow.getSurface(), DeviceName)
{
	LOG_DEBUG("VulkanApplication instance created");
	LOG_DEBUG("\tApplication name is " << appName);
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
