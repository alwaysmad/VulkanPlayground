#include "VulkanApplication.hpp"
#include "DebugOutput.hpp"

static constexpr uint64_t timeout = UINT64_MAX;

VulkanApplication::VulkanApplication(const std::string& AppName, const std::string& DeviceName, uint32_t w, uint32_t h) :
	appName(AppName),
	glfwContext(),
	vulkanInstance(appName, glfwContext),
	vulkanWindow(vulkanInstance.getInstance(), w, h, appName),
	vulkanDevice(vulkanInstance.getInstance(), vulkanWindow.getSurface(), DeviceName),
	vulkanSwapchain(vulkanDevice, vulkanWindow),
	vulkanPipeline(vulkanDevice, vulkanSwapchain),
	vulkanSync(vulkanDevice, MAX_FRAMES_IN_FLIGHT, vulkanSwapchain.getImages().size()),
	vulkanCommand(vulkanDevice, vulkanSwapchain, vulkanPipeline)
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

	// Frame counter
	size_t currentFrame = 0;
	
	while (!vulkanWindow.shouldClose())
	{
		vulkanWindow.pollEvents();

		// 1. Wait for the previous frame to finish
		// The fence signals when the GPU is done with this frame's command buffer
		auto& fence = vulkanSync.getInFlightFence(currentFrame);

		// Wait returns a result, usually Success. vk::raii throws on error.
		(void)vulkanDevice.device().waitForFences({*fence}, vk::True, timeout);

		// 2. Acquire image from swapchain
		// This semaphore signals when the image is actually ready to be drawn to
		auto& imageAvailableSem = vulkanSync.getImageAvailableSemaphore(currentFrame);

		vk::AcquireNextImageInfoKHR acquireInfo {
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
		catch (const vk::OutOfDateKHRError&) {
			vulkanSwapchain.recreate();
			vulkanSync.refresh(vulkanSwapchain.getImages().size());
			continue;
		}	

		// Only reset the fence if we are actually submitting work
		vulkanDevice.device().resetFences({*fence});

		// 3. Record Commands
		vulkanCommand.recordDraw(currentFrame, imageIndex);

		// 4. Submit to Graphics Queue
		auto& renderFinishedSem = vulkanSync.getRenderFinishedSemaphore(imageIndex);
		constexpr vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

		const vk::SubmitInfo submitInfo {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*imageAvailableSem,
			.pWaitDstStageMask = &waitStages,
			.commandBufferCount = 1,
			.pCommandBuffers = &*vulkanCommand.getBuffer(currentFrame),
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
			auto result = vulkanDevice.presentQueue().presentKHR(presentInfo);
			if (result == vk::Result::eSuboptimalKHR) { vulkanSwapchain.recreate(); }
		}
		catch (const vk::OutOfDateKHRError&) {
			vulkanSwapchain.recreate();
			vulkanSync.refresh(vulkanSwapchain.getImages().size());
		}
		// Advance frame
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	vulkanDevice.device().waitIdle();
	LOG_DEBUG("VulkanApplication instance successfully finished run()");
	return EXIT_SUCCESS;
}
