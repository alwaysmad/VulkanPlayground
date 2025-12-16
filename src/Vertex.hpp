#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/packing.hpp>
#include <array>
#include <algorithm>
#include <cmath>

struct Vertex
{
	// 1. Position + Var1 (Packed into 4x 16-bit SNORM) -> 8 bytes
	// Layout: [Pos.x, Pos.y, Pos.z, Var1]
	int16_t posVar1[4];

	// 2. Params + Var2 (Packed into 4x 16-bit SFLOAT) -> 8 bytes
	// Layout: [Param1, Param2, Param3, Var2]
	uint16_t paramsVar2[4];

	// --- Constructors ---

	// Default constructor (required for resizing vectors sometimes)
	Vertex() = default;

	// Optimized Constructor for emplace_back
	// Takes a reference to a 32-byte block (cache friendly)
	// Layout: { x, y, z, var1, p1, p2, p3, var2 }
	explicit Vertex(const std::array<float, 8>& data) 
		: posVar1{ packSnorm(data[0]), 
			   packSnorm(data[1]), 
			   packSnorm(data[2]), 
			   packSnorm(data[3]) },
		paramsVar2{ glm::packHalf1x16(data[4]), 
			    glm::packHalf1x16(data[5]), 
			    glm::packHalf1x16(data[6]), 
			    glm::packHalf1x16(data[7]) 
		}
	{}

	// --- Static Descriptors ---
	static constexpr vk::VertexInputBindingDescription bindingDescription {
		.binding = 0,
		.stride = sizeof(Vertex),
		.inputRate = vk::VertexInputRate::eVertex
	};

	static constexpr std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions {{
		{ 0, 0, vk::Format::eR16G16B16A16Snorm, offsetof(Vertex, posVar1) },
		{ 1, 0, vk::Format::eR16G16B16A16Sfloat, offsetof(Vertex, paramsVar2) }
	}};

private:
	static inline int16_t packSnorm(float f)
		{ return static_cast<int16_t>(std::round(std::clamp(f, -1.0f, 1.0f) * 32767.0f)); }
};
