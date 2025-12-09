// src/GlfwContext.cpp
#include "GlfwContext.hpp"
#include <stdexcept>
#include "DebugOutput.hpp"
#include <GLFW/glfw3.h>

GlfwContext::GlfwContext()
{
	if (glfwInit() != GLFW_TRUE) 
		throw std::runtime_error("Failed to initialize GLFW");
	LOG_DEBUG("GLFW Initialized");
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
}

GlfwContext::~GlfwContext()
{
	glfwTerminate();
	LOG_DEBUG("GLFW Terminated");
}

std::vector<const char*> GlfwContext::getRequiredInstanceExtensions() const
{
	uint32_t count = 0;
	const char** extensions = glfwGetRequiredInstanceExtensions(&count);
	return std::vector<const char*>(extensions, extensions + count);
}
