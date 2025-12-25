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
[[vk::binding(0, 0)]] 
ConstantBuffer<Satellite> satellites[512];

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

	// Bounds Check (Pseudo-code, assumes you pass count somewhere)
	// if (index >= pc.vertexCount) return;

	// 1. Read Raw Vertex
	ComputeVertex v = earthVertices[index];

	// 2. Manual Unpack (SNORM -> Float)
	// Max int16 is 32767. We divide to normalize to [-1.0, 1.0].
	float3 pos;
	pos.x = max(float(v.rawPos.x) / 32767.0, -1.0);
	pos.y = max(float(v.rawPos.y) / 32767.0, -1.0);
	pos.z = max(float(v.rawPos.z) / 32767.0, -1.0);

	// 3. Scale to World Unit (Earth Radius)
	float3 worldPos = pos * 6371.0;

	// --- PLACEHOLDER LOGIC ---
	// Example: Increase the "Red" channel intensity based on Y height
	float dummyScore = (worldPos.y > 0) ? 0.01 : 0.0;

	// 4. Update the Vertex Data (Manual Pack)
	// We add to the existing red channel (v.rawParams.x)
	// Note: rawParams is float16_t4, so we can do math directly
	v.rawParams.x += float16_t(dummyScore);

	// 5. Write Back
	earthVertices[index] = v;
}
