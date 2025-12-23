#pragma once

// Helper struct for 16-bit 4x4 Matrix (32 bytes)
struct PackedHalfMat4
{
	uint16_t data[16];
	PackedHalfMat4() = default;
	explicit PackedHalfMat4(const glm::mat4& m)
	{
		const float* f = &m[0][0]; // Pointer to first element (GLM is col-major)

		for (int i = 0; i < 16; i++)
		{
			// Convert float32 -> float16 (represented as uint16_t)
			data[i] = glm::packHalf1x16(f[i]);
		}
	}
};

struct CameraPushConstants
{
	PackedHalfMat4 viewProj; // (32 bytes)
	glm::mat4 model; // (64 bytes)
	// Total: 96 bytes
};
