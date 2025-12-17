#include "VulkanCommand.hpp"
#include "VulkanDevice.hpp"
#include "DebugOutput.hpp"

VulkanCommand::VulkanCommand(const VulkanDevice& device, uint32_t queueFamilyIndex)
	: m_device(device),
	  // 1. Create Pool for the SPECIFIC queue family
	  m_commandPool(
		  device.device(),
		  vk::CommandPoolCreateInfo{
			  .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			  .queueFamilyIndex = queueFamilyIndex // <--- Used here
		  }
	  ),
	  // 2. Allocate Buffers
	  m_commandBuffers(
		  device.device(),
		  vk::CommandBufferAllocateInfo{
			  .commandPool = m_commandPool,
			  .level = vk::CommandBufferLevel::ePrimary,
			  .commandBufferCount = MAX_FRAMES_IN_FLIGHT
		  }
	  )
{
	LOG_DEBUG("VulkanCommand resources created for Queue Family " << queueFamilyIndex);
}

VulkanCommand::~VulkanCommand()
{
	LOG_DEBUG("VulkanCommand resources destroyed");
}
