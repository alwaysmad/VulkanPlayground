#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include "Vertex.hpp"

class Mesh
{
public:
	// 1. CPU Data (Filled by you, e.g., via a generator)
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	// 2. GPU Data (Filled by Renderer::uploadMesh)
	vk::raii::Buffer vertexBuffer = nullptr;
	vk::raii::DeviceMemory vertexMemory = nullptr;

	vk::raii::Buffer indexBuffer = nullptr;
	vk::raii::DeviceMemory indexMemory = nullptr;

	// Helper to check if we are ready to draw
	[[nodiscard]] bool isUploaded() const { return vertexBuffer != nullptr; }
};
