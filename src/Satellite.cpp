#include "Satellite.hpp"
#include "VulkanDevice.hpp"

SatelliteNetwork::SatelliteNetwork(const VulkanDevice& device) 
	: m_device(device)
{
	// Pre-allocate vector capacity to avoid reallocations
	satellites.reserve(MAX_SATELLITES);

	createResources();
	LOG_DEBUG("SatelliteNetwork created");
}

SatelliteNetwork::~SatelliteNetwork()
{
	// RAII handles unmapping and destruction
	LOG_DEBUG("SatelliteNetwork destroyed");
}

void SatelliteNetwork::createResources()
{
	// Create UBO (Host Visible = CPU can write to it)
	auto [buf, mem] = m_device.createBuffer(
			requiredUBOsize,
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	m_buffer = std::move(buf);
	m_memory = std::move(mem);

	// Persistently Map Memory
	// We keep this pointer open for the lifetime of the object
	m_mappedPtr = m_memory->mapMemory(0, requiredUBOsize);
}

void SatelliteNetwork::upload()
{
	if (!m_mappedPtr) return;

	// Safety: Clamp to MAX_SATELLITES
	uint32_t count = std::min((uint32_t)satellites.size(), MAX_SATELLITES);

	// Map to the Container Struct layout
	SatelliteUBO* ubo = static_cast<SatelliteUBO*>(m_mappedPtr);

	// 1. Write Count
	ubo->count = static_cast<int>(count);

	// 2. Write Data
	if (count > 0)
		{ std::memcpy(ubo->satellites, satellites.data(), count * sizeof(SatelliteData)); }
}
