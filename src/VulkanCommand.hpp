#pragma once
#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;

class VulkanCommand
{
public:
	// Double buffering
	static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	// Helper to flip between 0 and 1
	static inline uint32_t advanceFrame(uint32_t currentFrame)
		{ return currentFrame ^ 1u; }

	// Change: Now requires the specific queue family index
	VulkanCommand(const VulkanDevice& device, uint32_t queueFamilyIndex);
	~VulkanCommand();

	const vk::raii::CommandBuffer& getBuffer(uint32_t index) const { return m_commandBuffers[index]; }
	const vk::raii::CommandPool& getPool() const { return m_commandPool; }

private:
	const VulkanDevice& m_device;
	vk::raii::CommandPool m_commandPool;
	vk::raii::CommandBuffers m_commandBuffers;
};
