#include "VulkanApplication.hpp"
#include "DebugOutput.hpp"

VulkanApplication::VulkanApplication() :
	context(),
	instance(context, createInfo)
{
	LOG_DEBUG("VulkanApplication instance created successfully");
}

int VulkanApplication::run()
{
	LOG_DEBUG("VulkanApplication instance started run()");
	return EXIT_SUCCESS;
}
