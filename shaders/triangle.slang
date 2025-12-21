// 1. Push Constants
struct PushConstants
{
	float4x4 viewProj;
};

[[vk::push_constant]]
PushConstants pc;

// 2. Vertex Input
// Matches src/Vertex.hpp attributes
struct VSInput
{
	// Location 0: posVar1 (x, y, z, var1)
	// Format: R16G16B16A16_SNORM -> Reads as floats in [-1.0, 1.0]
	[[vk::location(0)]] float4 posVar1;

	// Location 1: paramsVar2 (p1, p2, p3, var2)
	// Format: R16G16B16A16_SFLOAT -> Reads as standard floats
	[[vk::location(1)]] float4 paramsVar2;
};

// 3. Vertex Output
struct VSOutput
{
	float3 color : COLOR;
	float4 sv_position : SV_Position;
};

[shader("vertex")]
VSOutput vertMain(VSInput input)
{
	VSOutput output;

	// Position Logic:
	output.sv_position = mul(pc.viewProj, float4(input.posVar1.xyz, 1.0));

	// Color Logic:
	output.color = input.paramsVar2.rgb;

	return output;
}

[shader("fragment")]
float4 fragMain(VSOutput inVert) : SV_Target
{
	return float4(inVert.color, 1.0);
	//return float4(inVert.color * clamp( -1.5/(-3.3 + 0.1/ (1 - inVert.sv_position.z)) , 0.01, 1.0), 1.0);
}
