#include "Satellite.hpp"

SatelliteNetwork::SatelliteNetwork(const VulkanDevice& device, uint32_t count) 
	: m_device(device)
{
	// Yell at user if too many satellites
	if (count > MAX_SATELLITES)
		{ throw std::runtime_error("Satellite count is higher " + MAX_SATELLITES); }
	
	// Pre-allocate vector capacity to avoid reallocations
	satellites.reserve(count);
	m_bufferSize = count * sizeof(SatelliteData);

	// Create UBO (Host Visible = CPU can write to it)
	auto [buf, mem] = m_device.createBuffer(
			m_bufferSize,
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	m_buffer = std::move(buf);
	m_memory = std::move(mem);

	// Persistently Map Memory
	// We keep this pointer open for the lifetime of the object
	m_mappedPtr = m_memory->mapMemory(0, m_bufferSize);
	
	LOG_DEBUG("SatelliteNetwork created for " << count << " satellites capacity");
}

SatelliteNetwork::~SatelliteNetwork()
{
	// RAII handles unmapping and destruction
	LOG_DEBUG("SatelliteNetwork destroyed");
}

void SatelliteNetwork::upload()
{
	// Ignore safety checks
	// if (!m_mappedPtr || satellites.empty()) return;

	// Copy the raw vector data directly to the GPU buffer
	// We trust the user not to exceed the capacity set in constructor
	std::memcpy(m_mappedPtr, satellites.data(), satellites.size() * sizeof(SatelliteData));
}
