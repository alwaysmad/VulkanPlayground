#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "VulkanSwapchain.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanSync.hpp"
#include "VulkanCommand.hpp" // Now fully included because we store it by value
#include "Mesh.hpp"

class VulkanDevice;
class VulkanWindow;

class Renderer
{
public:
	Renderer(const VulkanDevice& device, const VulkanWindow& window);
	~Renderer();

	void uploadMesh(Mesh& mesh);
	void draw(const Mesh& mesh);

private:
	const VulkanDevice& m_device;
	const VulkanWindow& m_window;

	// Owned Resources
	VulkanCommand   m_command;   // <--- Renderer owns this now
	VulkanSwapchain m_swapchain;
	VulkanPipeline  m_pipeline;
	VulkanSync      m_sync;

	uint32_t m_currentFrame = 0;

	[[nodiscard]] std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> 
	uploadToDevice(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage);

	void recordCommands(const vk::raii::CommandBuffer& cmd, uint32_t imageIndex, const Mesh& mesh);
	void recreateSwapchain();
};
