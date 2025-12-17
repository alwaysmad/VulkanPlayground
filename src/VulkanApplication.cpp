#include "VulkanApplication.hpp"
#include "Mesh.hpp"
#include "DebugOutput.hpp"

VulkanApplication::VulkanApplication(const std::string& AppName, const std::string& DeviceName, uint32_t w, uint32_t h) :
	appName(AppName),
	glfwContext(),
	vulkanInstance(appName, glfwContext),
	vulkanWindow(vulkanInstance, w, h, appName),
	vulkanDevice(vulkanInstance, vulkanWindow, DeviceName),
	vulkanCommand(vulkanDevice, Renderer::MAX_FRAMES_IN_FLIGHT),
	renderer(vulkanDevice, vulkanWindow, vulkanCommand)
{
	LOG_DEBUG("VulkanApplication instance created");
}

VulkanApplication::~VulkanApplication()
{
	LOG_DEBUG("VulkanApplication instance destroyed");
}

int VulkanApplication::run()
{
	LOG_DEBUG("VulkanApplication started run()");

	// 1. GENERATE
	Mesh myMesh;
	myMesh.vertices = {
		Vertex(std::array<float, 8>{-0.5f, -0.5f, 0.0f, 0,  1, 0, 0, 0}), // 0
		Vertex(std::array<float, 8>{ 0.5f, -0.5f, 0.0f, 0,  0, 1, 0, 0}), // 1
		Vertex(std::array<float, 8>{ 0.5f,  0.5f, 0.0f, 0,  0, 0, 1, 0}), // 2
		Vertex(std::array<float, 8>{-0.5f,  0.5f, 0.0f, 0,  1, 1, 1, 0})  // 3
	};
	myMesh.indices = { 0, 1, 2, 2, 3, 0 }; 

	// 2. UPLOAD
	renderer.uploadMesh(myMesh);

	// 3. RUN
	while (!vulkanWindow.shouldClose())
	{
		vulkanWindow.pollEvents();
		vulkanWindow.updateFPS(appName);
		renderer.draw(myMesh);
	}
	// Wait for the GPU to finish executing the last frame
	// BEFORE 'myMesh' is destroyed at the closing brace.
	vulkanDevice.device().waitIdle();

	LOG_DEBUG("VulkanApplication ended run()");
	return EXIT_SUCCESS;
}
