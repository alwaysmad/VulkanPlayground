// shaders/shared.slang
#pragma once

// Packed Satellite Structure
// "camera" contains:
// - Upper 3x3: View Rotation
// - Col 3 (xyz): View Translation
// - Row 3 (w components): [Tan(Fov/2), Aspect, Near, Far]

// The data structure shared between CPU, Compute Shader, and Vertex Shader
struct Satellite
{
	float4x4 camera;
	float4 color;
};

struct SatelliteBlock
{
	Satellite satellites[512];
};

float4x4 getViewProj(float4x4 c)
{
	// Unpack camera paramaters
	float t = c[3].x; // Tan(FOV/2)
	float a = c[3].y; // Aspect
	float n = c[3].z; // z-Near
	float f = c[3].w; // z-Far
	
	// Recover View
	float4x4 view = c;

	view[3].x = 0.0;
	view[3].y = 0.0;
	view[3].x = 0.0;
	view[3].w = 1.0;

	// Construct Proj
	float4x4 proj = float4x4(
		1.0/(a*t),	0.0,	0.0,	0.0,
		0.0,	-1.0/(t),	0.0,	0.0,
		0.0,	0.0,	-f/(f-n),	-f*n/(f-n),
		0.0,	0.0,		-1.0,	0.0
	);

	return mul(proj, view);	
}

float4x4 getInvViewProj(float4x4 c)
{
	// Unpack camera paramaters
	float t = c[3].x; // Tan(FOV/2)
	float a = c[3].y; // Aspect
	float n = c[3].z; // z-Near
	float f = c[3].w; // z-Far
	
	// Construct inverse view matrix
	// Transpose Rotation + Dot Product Translation
	float4x4 iView = float4x4(
		float4(c[0].x, c[1].x, c[2].x, -c[0].w*c[0].x -c[1].w*c[1].x -c[2].w*c[2].x),
		float4(c[0].y, c[1].y, c[2].y, -c[0].w*c[0].y -c[1].w*c[1].y -c[2].w*c[2].y),
		float4(c[0].z, c[1].z, c[2].z, -c[0].w*c[0].z -c[1].w*c[1].z -c[2].w*c[2].z),
		float4(0.0, 0.0, 0.0, 1.0)
	);

	// Construct inverse proj matrix
	float4x4 iProj = float4x4(
		(a*t),	0.0,	0.0,		0.0,
		0.0,	-t,	0.0,		0.0,
		0.0,	0.0,	0.0,		-1.0,
		0.0,	0.0,	1.0/f - 1.0/n,	1.0/n
	);

	return mul(iView, iProj);
}
