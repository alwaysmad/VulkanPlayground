#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "VulkanCommand.hpp"
#include "VulkanDevice.hpp"
#include "DebugOutput.hpp"

class VulkanCommand
{
private:
	vk::raii::CommandPool m_commandPool;
	vk::raii::CommandBuffers m_commandBuffers;
public:
	// Double buffering
	static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	// Helper to flip between 0 and 1
	static inline uint32_t advanceFrame(uint32_t currentFrame)
		{ return currentFrame ^ 1u; }

	explicit VulkanCommand(const VulkanDevice& device, uint32_t count) :
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
};
