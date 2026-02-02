// shaders/satellite.slang
#include "shared.slang"

struct PushConstants
{
	half4x4 mainCameraViewProj;
};
[[vk::push_constant]] PushConstants pc;

[[vk::binding(0, 0)]] ConstantBuffer<SatelliteBlock> satBuffer;

struct VertexOutput
{
	float4 pos : SV_Position;
	float4 color : COLOR;
};

// 9 Points: 1 Tip + 8 Corners (Near/Far)
// Z=0 is Near, Z=1 is Far (Vulkan)
static const float4 kNdcCorners[9] = {
	float4(         0, 0, -1, 0),     // 0: Tip
	float4(-1, -1, 0, 1),    // 1: Near BL
	float4( 1, -1, 0, 1),    // 2: Near BR
	float4( 1,  1, 0, 1),    // 3: Near TR
	float4(-1,  1, 0, 1),    // 4: Near TL
	float4(-1, -1, 1, 1),    // 5: Far BL
	float4( 1, -1, 1, 1),    // 6: Far BR
	float4( 1,  1, 1, 1),    // 7: Far TR
	float4(-1,  1, 1, 1)     // 8: Far TL
};

// 16 Indices (Line List)
// Tip->Near + Near Loop + Near->Far + Far Loop
static const uint kIndices[32] = {
	// Pyramid (Tip to Near)
	0, 1,  0, 2,  0, 3,  0, 4,
	// Near Loop
	1, 2,  2, 3,  3, 4,  4, 1,
	// Frustum Sides (Near to Far)
	1, 5,  2, 6,  3, 7,  4, 8,
	// Far Loop
	5, 6,  6, 7,  7, 8,  8, 5
};

[shader("vertex")]
VertexOutput vertMain(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	// 1. Get Analytic Inverse Matrix
	// This is (View * Proj)^-1
	float4x4 invViewProj = getInvViewProj(satBuffer.satellites[instanceID].camera);
	//float4x4 invViewProj = getInvViewProj(satBuffer.satellites[0].camera);

	// 2. Vertex Pulling
	uint idx = kIndices[vertexID];
	float4 ndcPos = kNdcCorners[idx];

	// 3. Unproject to World Space
	float4 worldPos = mul(invViewProj, ndcPos);

	// Perspective Divide is automatic
	// For the tip (w=0 input), worldPos.w will be non-zero (1/n - 1/f)
	// Dividing by it places the vertex exactly at the camera origin.
	// worldPos /= worldPos.w; 

	// 4. Project to Main Camera
	VertexOutput out;
	out.pos = mul(pc.mainCameraViewProj, worldPos);
	//out.pos = mul(pc.mainCameraViewProj, ndcPos);
	out.color = satBuffer.satellites[instanceID].color;
	return out;
}

[shader("fragment")]
float4 fragMain(VertexOutput in) : SV_Target
{
	return in.color;
}
