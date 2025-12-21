#pragma once

struct CameraPushConstants
{
	glm::mat4 viewProj; // 0...63
	glm::mat4 model;    // 64...127
};
