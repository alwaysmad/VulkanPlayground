#include "VulkanApplication.hpp"
#include "Mesh.hpp"
#include "DebugOutput.hpp"
#include "VulkanCommand.hpp" // for MAX_FRAMES_IN_FLIGHT

VulkanApplication::VulkanApplication(const std::string& AppName, const std::string& DeviceName, uint32_t w, uint32_t h) :
	appName(AppName),
	glfwContext(),
	vulkanInstance(appName, glfwContext.getRequiredInstanceExtensions()),
	vulkanWindow(vulkanInstance, w, h, appName),
	vulkanDevice(vulkanInstance, vulkanWindow, DeviceName),
	vulkanLoader(vulkanDevice),
	renderer(vulkanDevice, vulkanWindow)
{
	// Create Fences (Signaled so we don't wait on first frame)
	constexpr vk::FenceCreateInfo fenceInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };

	for (uint32_t i = 0; i < VulkanCommand::MAX_FRAMES_IN_FLIGHT; ++i)
		{ m_inFlightFences.emplace_back(vulkanDevice.device(), fenceInfo); }
	LOG_DEBUG("VulkanApplication instance created");
}

VulkanApplication::~VulkanApplication()
{
	// Wait for GPU to finish all work
	// This protects members from being destroyed while still in use
	vulkanDevice.device().waitIdle();
	LOG_DEBUG("VulkanApplication instance destroyed");
}

void VulkanApplication::fillMesh()
{
	m_mesh.vertices = {
		Vertex(std::array<float, 8>{-0.5f, -0.5f, 0.0f, 0,  1, 0, 0, 0}), // 0 
		Vertex(std::array<float, 8>{ 0.5f, -0.5f, 0.0f, 0,  0, 1, 0, 0}), // 1
		Vertex(std::array<float, 8>{ 0.5f,  0.5f, 0.0f, 0,  0, 0, 1, 0}), // 2
		Vertex(std::array<float, 8>{-0.5f,  0.5f, 0.0f, 0,  1, 1, 1, 0}), // 3
	};
	m_mesh.indices = { 0, 1, 2, 2, 3, 0 }; 
}

int VulkanApplication::run()
{
	LOG_DEBUG("VulkanApplication started run()");

	// 1. Prepare Data
	fillMesh();
	vulkanLoader.uploadMesh(m_mesh);

	uint32_t currentFrame = 0;

	// 2. Loop
	while (!vulkanWindow.shouldClose())
	{
		vulkanWindow.pollEvents();
		vulkanWindow.updateFPS(appName);

		auto& fence = m_inFlightFences[currentFrame];
		
		if (vulkanDevice.device().waitForFences({*fence}, vk::True, UINT64_MAX) != vk::Result::eSuccess)
			{ throw std::runtime_error("Fence wait failed"); }

		// 1. Compute Step (Future)
		// ... compute(..., fence) ...

		// 2. Render Step (Optional)
		// Renderer now guarantees 'fence' is signaled even if it skips drawing.
		renderer.draw(m_mesh, currentFrame, *fence);
		
		// 3. Flow Guaranteed: Always advance
		currentFrame = VulkanCommand::advanceFrame(currentFrame);
	}
	return EXIT_SUCCESS;
}
