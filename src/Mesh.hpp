#pragma once

struct Vertex
{
	// 1. Position + Var1 (Packed into 4x 16-bit SNORM) -> 8 bytes
	int16_t posVar1[4];

	// 2. Params + Var2 (Packed into 4x 16-bit SFLOAT) -> 8 bytes
	uint16_t paramsVar2[4];

	// --- Constructors ---
	Vertex() = default;

	explicit Vertex(const std::array<float, 8>& data) : 
		posVar1{ packSnorm(data[0]), packSnorm(data[1]), packSnorm(data[2]), packSnorm(data[3]) },
		paramsVar2{ glm::packHalf1x16(data[4]), glm::packHalf1x16(data[5]), glm::packHalf1x16(data[6]), glm::packHalf1x16(data[7]) }
	{}

private:
	static inline int16_t packSnorm(float f)
		{ return static_cast<int16_t>(std::round(std::clamp(f, -1.0f, 1.0f) * 32767.0f)); }
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
	alignas(16) std::vector<Vertex> vertices;
	alignas(16) std::vector<uint32_t> indices;

	// 2. GPU Data
	// Declare Memory BEFORE Buffer.
	// Destruction happens in reverse order:
	TrackedDeviceMemory vertexMemory;
	vk::raii::Buffer vertexBuffer = nullptr;

	TrackedDeviceMemory indexMemory;
	vk::raii::Buffer indexBuffer = nullptr;
};
