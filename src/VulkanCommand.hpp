#pragma once
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;
class VulkanSwapchain;
class VulkanPipeline;

// Max number of frames that can be processed concurrently by the CPU and GPU.
// This size determines the size of all command buffers and sync objects arrays.
constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;

class VulkanCommand
{
public:
	VulkanCommand(const VulkanDevice& device,
	              const VulkanSwapchain& swapchain,
	              const VulkanPipeline& pipeline);
	~VulkanCommand();
	
	const vk::raii::CommandPool& getPool() const { return m_commandPool; }
	const vk::raii::CommandBuffer& getBuffer(uint32_t index) const { return m_commandBuffers[index]; }

	void recordDraw(size_t bufferIndex, size_t imageIndex ); 
private:
	// Stored references for drawing
	const VulkanSwapchain& m_swapchain;
	const VulkanPipeline& m_pipeline;

	vk::raii::CommandPool m_commandPool;
	vk::raii::CommandBuffers m_commandBuffers; // This is internally a std::vector
};
