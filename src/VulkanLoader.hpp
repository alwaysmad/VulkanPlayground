#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "VulkanDevice.hpp"
#include "VulkanCommand.hpp"
#include "Mesh.hpp"

class VulkanLoader
{
public:
	explicit VulkanLoader(const VulkanDevice& device);
	~VulkanLoader();

	void uploadMesh(Mesh& mesh);

private:
	const VulkanDevice& m_device;
	VulkanCommand m_command; 

	[[nodiscard]] std::pair<vk::raii::Buffer, TrackedDeviceMemory> 
	uploadToDevice(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage);
};
