#include "VulkanApplication.hpp"
#include "Mesh.hpp"
#include "DebugOutput.hpp"
#include "VulkanCommand.hpp" // for MAX_FRAMES

VulkanApplication::VulkanApplication(const std::string& AppName, const std::string& DeviceName, uint32_t w, uint32_t h) :
	appName(AppName),
	glfwContext(),
	vulkanInstance(appName, glfwContext),
	vulkanWindow(vulkanInstance, w, h, appName),
	vulkanDevice(vulkanInstance, vulkanWindow, DeviceName),
	renderer(vulkanDevice, vulkanWindow)
{
	// Create Fences (Signaled so we don't wait on first frame)
	constexpr vk::FenceCreateInfo fenceInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };

	for (uint32_t i = 0; i < VulkanCommand::MAX_FRAMES_IN_FLIGHT; ++i) {
		m_inFlightFences.emplace_back(vulkanDevice.device(), fenceInfo);
	}
	LOG_DEBUG("VulkanApplication instance created");
}

VulkanApplication::~VulkanApplication() {
	LOG_DEBUG("VulkanApplication instance destroyed");
}

int VulkanApplication::run()
{
	LOG_DEBUG("VulkanApplication started run()");

	Mesh myMesh;
	myMesh.vertices = {
		Vertex(std::array<float, 8>{-0.5f, -0.5f, 0.0f, 0,  1, 0, 0, 0}),
		Vertex(std::array<float, 8>{ 0.5f, -0.5f, 0.0f, 0,  0, 1, 0, 0}),
		Vertex(std::array<float, 8>{ 0.5f,  0.5f, 0.0f, 0,  0, 0, 1, 0}),
		Vertex(std::array<float, 8>{-0.5f,  0.5f, 0.0f, 0,  1, 1, 1, 0})
	};
	myMesh.indices = { 0, 1, 2, 2, 3, 0 }; 

	renderer.uploadMesh(myMesh);

	uint32_t currentFrame = 0;

	while (!vulkanWindow.shouldClose())
	{
		vulkanWindow.pollEvents();
		vulkanWindow.updateFPS(appName);

		// 1. CPU Wait
		auto& fence = m_inFlightFences[currentFrame];
		(void)vulkanDevice.device().waitForFences({*fence}, vk::True, UINT64_MAX);
		
		// Note: We DO NOT reset the fence here. 
		// renderer.draw() will reset it ONLY if it successfully acquires an image.
		
		// 2. Render
		// Pass fence to Renderer to signal when GPU is done
		renderer.draw(myMesh, currentFrame, *fence);

		currentFrame = VulkanCommand::advanceFrame(currentFrame);
	}
	
	vulkanDevice.device().waitIdle();
	return EXIT_SUCCESS;
}
