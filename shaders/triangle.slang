// =========================================================================
// SHARED RESOURCES (Push Constants)
// =========================================================================
struct PushConstants
{
	half4x4 viewProj; // 16-bit Matrix (Clip Space)
	float4x4 model;   // 32-bit Matrix (World Space)
};

[[vk::push_constant]]
PushConstants pc;

// =========================================================================
// GRAPHICS PIPELINE (Vertex + Fragment)
// =========================================================================

// 1. Graphics Input: Hardware Unpacked

// Matches src/Vertex.hpp attributes
struct VertexInput
{
	// location 0: posvar1 (x, y, z, var1)
	// format: r16g16b16a16_snorm
	// Input assembler automatically unpacks from int16_t4
	[[vk::location(0)]] float4 posVar1;
	
	// Location 1: paramsVar2 (p1, p2, p3, var2)
	// Format: R16G16B16A16_SFLOAT
	// Input assembler automatically unpacks from float16_t4
	[[vk::location(1)]] float4 paramsVar2;
};
struct VertexOutput
{
	float3 color : COLOR;
	float4 sv_position : SV_Position;
};

[shader("vertex")]
VertexOutput vertMain(VertexInput input)
{
	VertexOutput vo;

	// Position Logic:
	vo.sv_position = mul(pc.viewProj, mul(pc.model, float4(input.posVar1.xyz, 1.0)));

	// Color Logic:
	vo.color = input.paramsVar2.rgb;

	return vo;
}

[shader("fragment")]
float4 fragMain(VertexOutput v) : SV_Target
{
	return float4(v.color, 1.0);
	//return float4(inVert.color * clamp( -1.5/(-3.3 + 0.1/ (1 - inVert.sv_position.z)) , 0.01, 1.0), 1.0);
}

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
