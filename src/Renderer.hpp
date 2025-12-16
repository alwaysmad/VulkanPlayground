#pragma once
#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;
class VulkanSwapchain;
class VulkanPipeline;

class Renderer
{
public:
	Renderer(const VulkanDevice& device, 
		 const VulkanSwapchain& swapchain, 
		 const VulkanPipeline& pipeline);
	~Renderer();

	// The logic moved here. 
	// Notice we pass the 'cmd' in, rather than looking it up inside.
	void drawFrame(const vk::raii::CommandBuffer& cmd, uint32_t imageIndex);

private:
	const VulkanDevice& m_device;
	const VulkanSwapchain& m_swapchain;
	const VulkanPipeline& m_pipeline;
};
