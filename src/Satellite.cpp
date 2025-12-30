#include "Satellite.hpp"

SatelliteNetwork::SatelliteNetwork(const VulkanDevice& device, uint32_t count) 
	: m_device(device)
{
	// Yell at user if too many satellites
	if (count > MAX_SATELLITES)
		{ throw std::runtime_error("Satellite count is higher " + std::to_string(MAX_SATELLITES)); }

	////////////////////////////////////////////////////////////////////////////////
	// Check UBO Size Limits
	////////////////////////////////////////////////////////////////////////////////
	const auto props = device.physicalDevice().getProperties();
	const auto uboLimit = props.limits.maxUniformBufferRange;

	LOG_DEBUG("Device UBO Limit: " << uboLimit << " bytes");
	LOG_DEBUG("Required Satellite Buffer: " << requiredUBOsize << " bytes");

	if (uboLimit < requiredUBOsize)
	{
		// Yell at the user (Critical Error)
		std::cerr << DBG_COLOR_RED
			<< "Selected device '" << props.deviceName
			<< "' has insufficient Uniform Buffer space!" << std::endl;
		std::cerr << "\tLimit: " << uboLimit << " bytes" << std::endl;
		std::cerr << "\tRequired: " << requiredUBOsize << " bytes" <<  DBG_COLOR_RESET << std::endl;

		throw std::runtime_error("Device UBO limit too small for satellite data");
	}


	// Pre-allocate vector capacity to avoid reallocations
	satellites.reserve(count);
	satellites.resize(count);

	// 1. Calculate Aligned Size
	// Uniform Buffer offsets must be aligned to minUniformBufferOffsetAlignment (usually 256 bytes)
	const auto align = m_device.physicalDevice().getProperties().limits.minUniformBufferOffsetAlignment;
	m_frameSize = (requiredUBOsize + align - 1) & ~(align - 1); // Align up

	// 2. Allocate Total Size
	const vk::DeviceSize totalSize = m_frameSize * MAX_FRAMES_IN_FLIGHT;

	// Create UBO (Host Visible = CPU can write to it)
	auto [buf, mem] = m_device.createBuffer(
			totalSize,
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	m_buffer = std::move(buf);
	m_memory = std::move(mem);

	// Persistently Map Memory
	// We keep this pointer open for the lifetime of the object
	m_mappedPtr = m_memory->mapMemory(0, vk::WholeSize);
	
	LOG_DEBUG("SatelliteNetwork created for " << count << " satellites capacity");
}

SatelliteNetwork::~SatelliteNetwork()
{
	// RAII handles unmapping and destruction
	LOG_DEBUG("SatelliteNetwork destroyed");
}
