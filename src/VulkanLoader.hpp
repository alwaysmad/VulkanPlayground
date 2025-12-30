#pragma once
#include "VulkanDevice.hpp"
#include "VulkanCommand.hpp"
#include "Mesh.hpp"
#include "Satellite.hpp"

class VulkanLoader
{
public:
	explicit VulkanLoader(const VulkanDevice& device);
	~VulkanLoader();

	// Synchronous mesh upload and download (keeps waiting until idle)
	void uploadMesh(Mesh& mesh);
	void downloadMesh(Mesh& mesh);

	// Async upload for per-frame data
	// Records copy from src -> dst
	// Submits to Transfer Queue
	// Signals 'signalSemaphore' when done
	void uploadAsync(
			uint32_t currentFrame,
			const vk::raii::Buffer& src, vk::DeviceSize srcOffset,
			const vk::raii::Buffer& dst, vk::DeviceSize dstOffset,
			vk::DeviceSize size, vk::Semaphore signalSemaphore
	);

private:
	const VulkanDevice& m_device;
	VulkanCommand m_command; 

	[[nodiscard]] std::pair<vk::raii::Buffer, TrackedDeviceMemory> 
	uploadToDevice(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage);
};
