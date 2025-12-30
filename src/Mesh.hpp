#pragma once
#include "VulkanDevice.hpp"

class VulkanLoader;

struct alignas(16) Vertex
{
	// 1. x, y, z, phi 
	// Packed into 4x 16-bit SNORM -> 8 bytes
	int16_t posVar1[4];

	// 2. a, V, SW, Omega
	// Packed into 4x 16-bit SFLOAT -> 8 bytes
	uint16_t paramsVar2[4];

	// --- Constructors ---
	Vertex() = default;
	explicit Vertex(const std::array<float, 8>& data);
};

class Mesh
{
public:
	static constexpr vk::VertexInputBindingDescription bindingDescription {
		.binding = 0,
		.stride = sizeof(Vertex),
		.inputRate = vk::VertexInputRate::eVertex
	};

	static constexpr std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions {{
		{ 0, 0, vk::Format::eR16G16B16A16Snorm, offsetof(Vertex, posVar1) },
		{ 1, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(Vertex, paramsVar2) }
	}};

	// 1. CPU Data
	std::vector<Vertex> vertices;
	alignas(16) std::vector<uint32_t> indices;

	explicit Mesh();
	~Mesh();

	// Self-Upload/Download
	void upload(VulkanLoader& loader);
	void download(VulkanLoader& loader);

	// Getters for binding
	const inline vk::raii::Buffer& getVertexBuffer() { return vertexBuffer; }
	const inline vk::raii::Buffer& getIndexBuffer() { return indexBuffer; }
private:
	// 2. GPU Data
	// Declare Memory BEFORE Buffer.
	// Destruction happens in reverse order:
	TrackedDeviceMemory vertexMemory;
	vk::raii::Buffer vertexBuffer = nullptr;

	TrackedDeviceMemory indexMemory;
	vk::raii::Buffer indexBuffer = nullptr;
};
