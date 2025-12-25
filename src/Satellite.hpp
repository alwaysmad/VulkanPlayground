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
// This matches the shader's "Satellite satellites[512]" definition.
// 512 * 80 bytes = 40,960 bytes (well under 64KB limit).

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
	inline void upload(uint32_t currentFrame)
	{
		// Calculate where to write for THIS frame
		const size_t offset = currentFrame * m_frameSize;
		char* dst = static_cast<char*>(m_mappedPtr) + offset;

		// Write only the active data
		std::memcpy(dst, satellites.data(), satellites.size() * sizeof(SatelliteData));
	}

	// 3. Getters for Descriptor Binding
	inline const vk::raii::Buffer& getBuffer() const { return m_buffer; }

	// Helper to get the aligned size per frame
	inline vk::DeviceSize getFrameSize() const { return m_frameSize; }
private:
	const VulkanDevice& m_device;

	// GPU Resources
	vk::raii::Buffer m_buffer = nullptr;
	TrackedDeviceMemory m_memory;

	// Pointer to mapped GPU memory (Host Visible)
	void* m_mappedPtr = nullptr;

	vk::DeviceSize m_frameSize; // Size of one frame's data (aligned)
};
