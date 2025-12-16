#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "VulkanSwapchain.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanSync.hpp"
#include "Mesh.hpp"

class VulkanDevice;
class VulkanWindow;
class VulkanCommand;

class Renderer
{
public:
	// Renderer now needs Window to create the Swapchain internally
	Renderer(	const VulkanDevice& device,
			const VulkanWindow& window,
			const VulkanCommand& command );
	~Renderer();

	// 1. Resource Management
	void uploadMesh(Mesh& mesh);

	// 2. Main Loop: Handles Acquire -> Record -> Submit -> Present
	void draw(const Mesh& mesh);

private:
	const VulkanDevice& m_device;
	const VulkanCommand& m_command;
	const VulkanWindow& m_window; // Stored for recreation events

	// Owned Resources
	VulkanSwapchain m_swapchain;
	VulkanPipeline  m_pipeline;
	VulkanSync      m_sync;

	uint32_t m_currentFrame = 0;

	// Helper to deduplicate Staging -> Device copy logic
	[[nodiscard]] std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> 
	uploadToDevice(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage);

	// Internal record function
	void recordCommands(const vk::raii::CommandBuffer& cmd, uint32_t imageIndex, const Mesh& mesh);
	
	// Helper to handle window resize/minimization
	void recreateSwapchain();
};
