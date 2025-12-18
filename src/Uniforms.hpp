#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct CameraPushConstants
{
	glm::mat4 view;
	glm::mat4 proj;
};
