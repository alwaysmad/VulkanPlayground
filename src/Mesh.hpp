#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include "Vertex.hpp"

class Mesh
{
public:
	// 1. CPU Data
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	// 2. GPU Data
	// Declare Memory BEFORE Buffer.
	// Destruction happens in reverse order:
	vk::raii::DeviceMemory vertexMemory = nullptr;
	vk::raii::Buffer vertexBuffer = nullptr;

	vk::raii::DeviceMemory indexMemory = nullptr;
	vk::raii::Buffer indexBuffer = nullptr;

	[[nodiscard]] inline bool isUploaded() const { return vertexBuffer != nullptr; }
};
