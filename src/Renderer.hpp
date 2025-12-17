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

	void uploadMesh(Mesh& mesh);

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

	// Keep this one fixed size [MAX_FRAMES_IN_FLIGHT]
	std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores;

	// CHANGE: This one must match Swapchain Image Count!
	std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;

	[[nodiscard]] std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> 
	uploadToDevice(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage);

	void recordCommands(const vk::raii::CommandBuffer& cmd, uint32_t imageIndex, const Mesh& mesh);
	void recreateSwapchain();
};
