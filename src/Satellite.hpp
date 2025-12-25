#pragma once
#include "VulkanDevice.hpp"

// The fixed capacity for our solver
constexpr uint32_t MAX_SATELLITES = 512;

struct SatelliteData
{
	// 1. Matrix (64 bytes)
	// alignas(16) is implicit for mat4, but good to be explicit for GPU data
	alignas(16) glm::mat4 viewProj; 

	// 2. Intensity/Data (16 bytes)
	alignas(16) float data[4];
};

// Calculate required size
constexpr vk::DeviceSize requiredUBOsize = sizeof(SatelliteData) * MAX_SATELLITES;

class SatelliteNetwork
{
public:
	// 1. CPU Data
	// Modify this vector directly, then call upload()
	std::vector<SatelliteData> satellites;

	explicit SatelliteNetwork(const VulkanDevice& device, uint32_t maxCount = MAX_SATELLITES);
	~SatelliteNetwork();

	// 2. GPU Sync
	// Copies the current 'satellites' vector to the UBO.
	// Safe to call every frame.
	void upload();

	// 3. Getters for Descriptor Binding
	inline const vk::raii::Buffer& getBuffer() const { return m_buffer; }
	inline vk::DeviceSize getSize() const { return m_bufferSize; }

private:
	const VulkanDevice& m_device;

	// GPU Resources
	vk::raii::Buffer m_buffer = nullptr;
	TrackedDeviceMemory m_memory;

	// Pointer to mapped GPU memory (Host Visible)
	void* m_mappedPtr = nullptr;

	vk::DeviceSize m_bufferSize;
};
