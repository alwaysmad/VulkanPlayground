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

float4x4 getViewProj(float4x4 camera)
{
	// Unpack camera paramaters
	float t = camera[0].w; // Tan(FOV/2)
	float a = camera[1].w; // Aspect
	float n = camera[2].w; // z-Near
	float f = camera[3].w; // z-Far
	
	// Recover View
	float4x4 view = camera;

	view[0].w = 0.0;
	view[1].w = 0.0;
	view[2].w = 0.0;
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

float4x4 getInvViewProj(float4x4 camera)
{
	// Unpack camera paramaters
	float t = 0.5; //camera[0].w; // Tan(FOV/2)
	float a = 1; //camera[1].w; // Aspect
	float n = 0.1; //camera[2].w; // z-Near
	float f = 0.4; //camera[3].w; // z-Far
	
	// Construct inverse view matrix
	// Transpose Rotation + Dot Product Translation
	float4x4 iView = float4x4(
		float4(camera[0].xyz, -dot(camera[0].xyz, camera[3].xyz)),
		float4(camera[1].xyz, -dot(camera[1].xyz, camera[3].xyz)),
		float4(camera[2].xyz, -dot(camera[2].xyz, camera[3].xyz)),
		float4(0.0, 0.0, 0.0, 1.0)
	);
	iView = camera;

	// Construct inverse proj matrix
	float4x4 iProj = float4x4(
		(a*t),	0.0,	0.0,		0.0,
		0.0,	-t,	0.0,		0.0,
		0.0,	0.0,	0.0,		-1.0,
		0.0,	0.0,	1.0/f - 1.0/n,	1.0/n
	);

	return mul(iView, iProj);
}
