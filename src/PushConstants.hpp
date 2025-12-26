#pragma once

// Helper struct for 16-bit 4x4 Matrix (32 bytes)
struct PackedHalfMat4
{
	uint16_t data[16];
	PackedHalfMat4() = default;
	explicit PackedHalfMat4(const glm::mat4& m)
	{
		const float* f = &m[0][0]; // Pointer to first element (GLM is col-major)

		// Convert float32 -> float16 (represented as uint16_t)
		for (int i = 0; i < 16; i++)
			{ data[i] = glm::packHalf1x16(f[i]); }
	}
};

struct CameraPushConstants
{
	PackedHalfMat4 viewProj; // (32 bytes)
	glm::mat4 model; // (64 bytes)
	// Total: 96 bytes
};

struct ComputePushConstants
{
	glm::mat4 modelMatrix;   // 64 bytes (To transform main mesh)
	uint32_t vertexCount;    // 4 bytes
	uint32_t satelliteCount; // 4 bytes
	float deltaTime;         // 4 bytes
	float padding;           // 4 bytes (Align to 80 bytes total)
};
