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

	// Returns 'true' if the frame was successfully submitted.
	// Returns 'false' if swapchain was recreated (and fence was NOT reset).
	bool draw(const Mesh& mesh, uint32_t currentFrame, const vk::Fence& fence, const vk::Semaphore* waitSemaphore = nullptr);

private:
	const VulkanDevice& m_device;
	const VulkanWindow& m_window;

	// Owned Resources
	VulkanCommand   m_command;
	VulkanSwapchain m_swapchain;
	VulkanPipeline  m_pipeline;

	// Sync
	// Keep this one fixed size [MAX_FRAMES_IN_FLIGHT]
	std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores;
	// This one must match Swapchain Image Count
	std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
	void remakeRenderFinishedSemaphores();
	void recordCommands(const vk::raii::CommandBuffer& cmd, uint32_t imageIndex, const Mesh& mesh);
	void recreateSwapchain();

	static constexpr std::array<float, 4> backgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};
};
