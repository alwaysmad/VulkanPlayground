// =========================================================================
// COMPUTE PIPELINE (Solver)
// =========================================================================

// 2. Compute Input: Raw Binary
// This matches the C++ 'Vertex' struct binary layout exactly.
struct ComputeVertex
{
	int16_t4 rawPos;      // The SNORM data (x, y, z, w)
	float16_t4 rawParams; // The SFLOAT data (r, g, b, a)
};

struct Satellite 
{ 
	float4x4 viewProj;
	float intensity;
	float3 padding;
};

// The Wrapper Struct
// This matches the single UBO blob sent from C++
struct SatelliteBlock
{
	Satellite satellites[512];
};

// --- Push Constants ---
struct PushConstants
{
	uint satelliteCount;
	uint vertexCount;
};
[[vk::push_constant]]
PushConstants pc;

// BINDINGS
// Binding 0: Satellites (Read-Only Uniforms)
// Bind the Wrapper, NOT the array directly
// "ConstantBuffer" = UBO. It will load the whole 40KB block.
[[vk::binding(0, 0)]] 
ConstantBuffer<SatelliteBlock> block;

// Binding 1: Earth Mesh (Read-Write Storage)
// This is the SAME buffer as VertexInput, but viewed as raw struct array
[[vk::binding(1, 0)]] 
RWStructuredBuffer<ComputeVertex> earthVertices;

[shader("compute")]
[numthreads(256, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
	return;
	uint index = id.x;

	if (index >= pc.vertexCount) return;

	ComputeVertex v = earthVertices[index];

	// Unpack (Manual SNORM to Float)
	float3 pos = float3(
		float(v.rawPos.x) / 32767.0, 
		float(v.rawPos.y) / 32767.0, 
		float(v.rawPos.z) / 32767.0
	);
	float3 worldPos = pos * 1.0;

	// FIX 4: Visualization Logic (Stripes)
	// We use assignment (=) instead of addition (+=) to prevent infinite growth
	float dummyScore = 0.0;

	// Simple test: Use the first satellite's data if available
	if (pc.satelliteCount > 0)
	{
		// Access array via the 'block' wrapper
		Satellite s = block.satellites[0];
		if (worldPos.y > 0) dummyScore = s.intensity; 
	}
	else
	{
		// Fallback pattern if no satellites uploaded yet
		if (worldPos.y > 0) dummyScore = 1.0;
	}

	// Write to Red channel
	v.rawParams.x = float16_t(dummyScore);

	earthVertices[index] = v;
}
