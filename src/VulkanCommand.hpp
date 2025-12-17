#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "VulkanCommand.hpp"
#include "VulkanDevice.hpp"
#include "DebugOutput.hpp"

class VulkanCommand
{
public:
	explicit VulkanCommand(const VulkanDevice& device, uint32_t count) :
		m_device(device),
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
				.commandBufferCount = count
			}
		)
		{ LOG_DEBUG("VulkanCommand resources created (" << m_commandBuffers.size() << " buffers)"); }
	~VulkanCommand()
		{ LOG_DEBUG("VulkanCommand resources destroyed"); }

	const vk::raii::CommandBuffer& getBuffer(uint32_t index) const { return m_commandBuffers[index]; }
	const vk::raii::CommandPool& getPool() const { return m_commandPool; }

private:
	const VulkanDevice& m_device;
	vk::raii::CommandPool m_commandPool;
	vk::raii::CommandBuffers m_commandBuffers;
};
