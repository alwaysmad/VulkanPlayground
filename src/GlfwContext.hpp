// src/GlfwContext.hpp
#pragma once
#include <vector>

class GlfwContext
{
public:
	GlfwContext();
	~GlfwContext();
	
	std::vector<const char*> getRequiredInstanceExtensions() const;
};
