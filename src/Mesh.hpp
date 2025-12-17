#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include "Vertex.hpp"

class Mesh
{
public:
	// 1. CPU Data
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// 2. GPU Data
	// Declare Memory BEFORE Buffer.
	// Destruction happens in reverse order:
	TrackedDeviceMemory vertexMemory;
	vk::raii::Buffer vertexBuffer = nullptr;

	TrackedDeviceMemory indexMemory;
	vk::raii::Buffer indexBuffer = nullptr;
};
