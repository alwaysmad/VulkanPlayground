#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "VulkanSwapchain.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanCommand.hpp" 
#include "Mesh.hpp"

class VulkanDevice;
class VulkanWindow;

class Renderer
{
public:
	Renderer(const VulkanDevice& device, const VulkanWindow& window);
	~Renderer();

	// - Returns void. The App controls the frame loop.
	// - Guarantees 'fence' is signaled, even if drawing fails/skips.
	//
	// SYNC OBJECTS:
	// - fence (Mandatory): 
	//     Signaled by GPU when this frame's submission completes. 
	//     Used by CPU (App) to wait before re-using resources (like CommandBuffers).
	//
	// - waitSemaphore (Optional): 
	//     If provided (not null), the Graphics Queue will wait for this semaphore 
	//     (at Vertex Input stage) before processing. Used to sync Compute -> Graphics.
	void draw(const Mesh& mesh, uint32_t currentFrame, vk::Fence fence, vk::Semaphore waitSemaphore = {});

private:
	const VulkanDevice& m_device;
	const VulkanWindow& m_window;

	// Owned Resources
	VulkanCommand   m_command;
	VulkanSwapchain m_swapchain;
	VulkanPipeline  m_pipeline;

	// Sync
	// Fixed size [MAX_FRAMES_IN_FLIGHT]
	std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores;
	// This one must match Swapchain Image Count
	std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
	void remakeRenderFinishedSemaphores();
	
	void recordCommands(const vk::raii::CommandBuffer& cmd, uint32_t imageIndex, const Mesh& mesh);
	void recreateSwapchain();
	void submitDummy(vk::Fence fence, vk::Semaphore waitSemaphore);

	static constexpr std::array<float, 4> backgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};
};
