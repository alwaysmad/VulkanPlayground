#pragma once
#include <glm/glm.hpp>

// The fixed capacity for our solver
constexpr uint32_t MAX_SATELLITES = 512;

struct SatelliteData
{
	// 1. Matrix (64 bytes)
	// alignas(16) is implicit for mat4, but good to be explicit for GPU data
	alignas(16) glm::mat4 viewProj; 

	// 2. Data (16 bytes)
	alignas(16) float data[4];
};

// Calculate required size
constexpr vk::DeviceSize requiredUBOsize = MAX_SATELLITES * sizeof(SatelliteData);
