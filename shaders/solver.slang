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
	float4 color;
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
	float4x4 modelMatrix; // Transform matrix of the Earth
	uint vertexCount;
	uint satelliteCount;
	float deltaTime;
	float padding;
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
	uint index = id.x;
	if (index >= pc.vertexCount) return;

	// 1. Direct Mapping: Vertex[i] gets Color from Satellite[i]
	// Safety check in case we have fewer satellites than vertices
	float4 newColor = float4(1.0, 1.0, 1.0, 1.0); // Default White

	if (index < pc.satelliteCount)
	{
		newColor = block.satellites[index].color;
	}

	// 2. Write to Vertex
	ComputeVertex v = earthVertices[index];

	// Cast float4 -> float16_t4
	v.rawParams = float16_t4(newColor);

	earthVertices[index] = v;
}
