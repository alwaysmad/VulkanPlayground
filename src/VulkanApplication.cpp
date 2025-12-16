#include "VulkanApplication.hpp"
#include "DebugOutput.hpp"

static constexpr uint64_t timeout = UINT64_MAX;

VulkanApplication::VulkanApplication(const std::string& AppName, const std::string& DeviceName, uint32_t w, uint32_t h) :
	appName(AppName),
	glfwContext(),
	vulkanInstance(appName, glfwContext),
	vulkanWindow(vulkanInstance, w, h, appName),
	vulkanDevice(vulkanInstance, vulkanWindow, DeviceName),
	vulkanSwapchain(vulkanDevice, vulkanWindow),
	vulkanPipeline(vulkanDevice, vulkanSwapchain),
	vulkanSync(vulkanDevice, VulkanCommand::MAX_FRAMES_IN_FLIGHT, vulkanSwapchain.getImages().size()),
	// Initialize Command System (Resource Manager)
	vulkanCommand(vulkanDevice),
	// Initialize Renderer (Logic)
	renderer(vulkanDevice, vulkanSwapchain, vulkanPipeline)
{
	LOG_DEBUG("VulkanApplication instance created");
	LOG_DEBUG("\tApplication name is " << appName);
}

VulkanApplication::~VulkanApplication()
{
	LOG_DEBUG("VulkanApplication instance destroyed");
}

int VulkanApplication::run()
{
	LOG_DEBUG("VulkanApplication instance started run()");

	uint32_t currentFrame = 0;
	// --- FPS Counter Variables ---
	double lastTime = vulkanWindow.getTime();
	uint32_t nbFrames = 0;

	while (!vulkanWindow.shouldClose())
	{
		// --- FPS Logic ---
		double currentTime = vulkanWindow.getTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0)
		{
			std::string title = appName + " - " + std::to_string(nbFrames) + " FPS";
			vulkanWindow.setWindowTitle(title);
			nbFrames = 0;
			lastTime = currentTime;
		}
		vulkanWindow.pollEvents();

		// 1. Wait for Fence
		auto& fence = vulkanSync.getInFlightFence(currentFrame);
		(void)vulkanDevice.device().waitForFences({*fence}, vk::True, timeout);

		// 2. Acquire Image
		auto& imageAvailableSem = vulkanSync.getImageAvailableSemaphore(currentFrame);

		const vk::AcquireNextImageInfoKHR acquireInfo {
			.swapchain = *vulkanSwapchain.getSwapchain(),
			.timeout = timeout,
			.semaphore = *imageAvailableSem,
			.fence = nullptr,
			.deviceMask = 1
		};

		uint32_t imageIndex;
		try {
			const auto result = vulkanDevice.device().acquireNextImage2KHR(acquireInfo);
			imageIndex = result.second;
		} 
		catch (const vk::OutOfDateKHRError&)
			{ refreshSwapchain(); continue; }

		vulkanDevice.device().resetFences({*fence});

		// 3. Record Commands (The New Way)
		// Get the raw buffer from the Command System
		const auto& cmd = vulkanCommand.getBuffer(currentFrame);
		
		// Tell the Renderer to draw into it
		renderer.drawFrame(cmd, imageIndex);

		// 4. Submit
		auto& renderFinishedSem = vulkanSync.getRenderFinishedSemaphore(imageIndex);
		constexpr vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

		const vk::SubmitInfo submitInfo {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*imageAvailableSem,
			.pWaitDstStageMask = &waitStages,
			.commandBufferCount = 1,
			.pCommandBuffers = &*cmd, // Use the cmd we just recorded
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &*renderFinishedSem
		};

		vulkanDevice.graphicsQueue().submit(submitInfo, *fence);

		// 5. Present
		const vk::PresentInfoKHR presentInfo {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*renderFinishedSem,
			.swapchainCount = 1,
			.pSwapchains = &*vulkanSwapchain.getSwapchain(),
			.pImageIndices = &imageIndex
		};

		try {
			const auto result = vulkanDevice.presentQueue().presentKHR(presentInfo);
			if (result == vk::Result::eSuboptimalKHR) { refreshSwapchain(); }
		}
		catch (const vk::OutOfDateKHRError&) 
			{ refreshSwapchain(); continue; }

		currentFrame = VulkanCommand::advanceFrame(currentFrame);
	}

	vulkanDevice.device().waitIdle();
	LOG_DEBUG("VulkanApplication instance successfully finished run()");
	return EXIT_SUCCESS;
}
