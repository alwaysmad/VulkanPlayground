#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "VulkanDevice.hpp"
#include "VulkanCommand.hpp"
#include "Mesh.hpp"

class VulkanLoader
{
public:
	// We pass the specific queue family index (Transfer)
	explicit VulkanLoader(const VulkanDevice& device);
	~VulkanLoader();

	void uploadMesh(Mesh& mesh);

private:
	const VulkanDevice& m_device;
	
	// Owns a Command Pool targeting the Transfer Queue
	VulkanCommand m_command; 

	[[nodiscard]] std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> 
	uploadToDevice(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage);
};
