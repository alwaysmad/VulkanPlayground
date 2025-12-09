#pragma once
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;
class VulkanSwapchain;
class VulkanPipeline;



class VulkanRender
{
public:
	// Max number of frames that can be processed concurrently by the CPU and GPU.
	// This size determines the size of all command buffers and sync objects arrays.
	static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
	static inline uint32_t advanceFrame (uint32_t currentFrame)
	{
		return currentFrame ^ 1u;
	}
	
	VulkanRender(const VulkanDevice& device, const VulkanSwapchain& swapchain);
	~VulkanRender();
	
	const vk::raii::CommandPool& getPool() const { return m_commandPool; }
	const vk::raii::CommandBuffer& getBuffer(uint32_t index) const { return m_commandBuffers[index]; }

	void recordDraw(uint32_t bufferIndex, uint32_t imageIndex, const VulkanPipeline& pipeline); 
private:
	// Stored references for drawing
	const VulkanSwapchain& m_swapchain;

	vk::raii::CommandPool m_commandPool;
	vk::raii::CommandBuffers m_commandBuffers; // This is internally a std::vector
};
