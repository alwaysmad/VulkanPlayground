#include "VulkanCommand.hpp"
#include "VulkanDevice.hpp"
#include "DebugOutput.hpp"

VulkanCommand::VulkanCommand(const VulkanDevice& device)
	: m_device(device),
	  // 1. Create Pool
	  m_commandPool(
		  device.device(),
		  vk::CommandPoolCreateInfo{
			  .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			  .queueFamilyIndex = device.getGraphicsQueueIndex()
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
	LOG_DEBUG("VulkanCommand resources created (" << m_commandBuffers.size() << " buffers)");
}

VulkanCommand::~VulkanCommand()
{
	LOG_DEBUG("VulkanCommand resources destroyed");
}
