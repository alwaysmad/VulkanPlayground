// src/GlfwContext.hpp
#pragma once

class GlfwContext
{
public:
	GlfwContext()
	{
		if (glfwInit() != GLFW_TRUE) 
			{ throw std::runtime_error("Failed to initialize GLFW"); }
		LOG_DEBUG("GLFW Initialized");
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	}

	~GlfwContext()
		{ glfwTerminate(); LOG_DEBUG("GLFW Terminated"); }
	
	inline std::vector<const char*> getRequiredInstanceExtensions() const
	{
		uint32_t count = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&count);
		return std::vector<const char*>(extensions, extensions + count);
	}
};
