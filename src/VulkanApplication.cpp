#include "VulkanApplication.hpp"
#include "Mesh.hpp"
#include "DebugOutput.hpp"
#include "VulkanCommand.hpp" // for MAX_FRAMES

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

VulkanApplication::~VulkanApplication() { LOG_DEBUG("VulkanApplication instance destroyed"); }

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

	vulkanLoader.uploadMesh(myMesh);

	uint32_t currentFrame = 0;

	while (!vulkanWindow.shouldClose())
	{
		vulkanWindow.pollEvents();
		vulkanWindow.updateFPS(appName);

		// 1. CPU Wait
		auto& fence = m_inFlightFences[currentFrame];
		
		// If we are about to submit to this frame slot, we must wait for it to be free.
		// However, if we skip submission (draw returns false), we didn't use the slot,
		// so we shouldn't wait/reset blindly next time unless we tracked that.
		// BUT: waitForFences on a signaled fence is instant, so it's safe to call.
		if (vulkanDevice.device().waitForFences({*fence}, vk::True, UINT64_MAX) != vk::Result::eSuccess)
			{ throw std::runtime_error("Fence wait failed"); }

		// 2. Render
		// We pass the fence. draw() will reset it ONLY if it submits.
		if (renderer.draw(myMesh, currentFrame, *fence))
		{
			// SUCCESS: The fence is now unsignaled (GPU working).
			// We can move to the next frame slot.
			currentFrame = VulkanCommand::advanceFrame(currentFrame);
		}
		else
		{
			// FAILURE (Swapchain Recreated): 
			// The fence was NOT reset (still signaled). 
			// We DO NOT advance currentFrame.
			// Next loop iteration will check the same fence (instant success) and retry draw().
		}
	}
	
	vulkanDevice.device().waitIdle();
	return EXIT_SUCCESS;
}
