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
