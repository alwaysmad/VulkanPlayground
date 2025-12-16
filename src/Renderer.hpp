#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "Mesh.hpp" // Add this include

class VulkanDevice;
class VulkanSwapchain;
class VulkanPipeline;
class VulkanCommand;

class Renderer
{
public:
	Renderer(const VulkanDevice& device, const VulkanSwapchain& swapchain,
		 const VulkanPipeline& pipeline, const VulkanCommand& command);
		
	~Renderer();
	
	// 1. Setup Phase: Uploads data using a temporary command buffer
	void uploadMesh(Mesh& mesh);

	// 2. Loop Phase: Records the frame
	void drawFrame(const vk::raii::CommandBuffer& cmd, uint32_t imageIndex, const Mesh& mesh);

	private:
	const VulkanDevice& m_device;
	const VulkanSwapchain& m_swapchain;
	const VulkanPipeline& m_pipeline;
	const VulkanCommand& m_command; // <--- Store it
};
