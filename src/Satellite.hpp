#pragma once
#include "VulkanDevice.hpp"

// The fixed capacity for our solver
constexpr uint32_t MAX_SATELLITES = 512;

struct alignas(16) SatelliteData
{
	// 1. Matrix (64 bytes)
	// View matrix + camera parameters: tanHalfFov, aspect, zNear, zFar 
	// [0][3] = tanHalfFov
	// [1][3] = aspect
	// [2][3] = zNear
	// [3][3] = zFar
	glm::mat4 camera;

	// 2. Intensity/Data (16 bytes)
	float data[4];
};

// Calculate required size
constexpr vk::DeviceSize requiredUBOsize = sizeof(SatelliteData) * MAX_SATELLITES;
// This matches the shader's  definition.
// 512 * 80 bytes = 40,960 bytes (well under 64KB limit).

class VulkanLoader;

class SatelliteNetwork
{
public:
	// 1. CPU Data
	// Modify this vector directly
	std::vector<SatelliteData> satellites;

	explicit SatelliteNetwork(const VulkanDevice& device, uint32_t maxCount = MAX_SATELLITES);
	~SatelliteNetwork();

	// 2. GPU Sync
	// Copies the current 'satellites' vector to the UBO.
	void upload(uint32_t currentFrame, VulkanLoader& loader, vk::Semaphore signalSemaphore);
	
	// 3. Getters for Descriptor Binding
	inline const vk::raii::Buffer& getBuffer() const { return m_deviceBuffer; }

	// Helper to get the aligned size per frame
	inline vk::DeviceSize getFrameSize() const { return m_frameSize; }
private:
	const VulkanDevice& m_device;

	// 1. GPU Buffer (Single, Device Local)
	vk::raii::Buffer m_deviceBuffer = nullptr;
	TrackedDeviceMemory m_deviceMemory;

	// 2. Staging Buffer (Ring, Host Visible)
	vk::raii::Buffer m_stagingBuffer = nullptr;
	TrackedDeviceMemory m_stagingMemory;
	// Pointer to mapped GPU memory (Host Visible)
	void* m_mappedPtr = nullptr;

	vk::DeviceSize m_frameSize; // Size of one frame's data (aligned)
};
