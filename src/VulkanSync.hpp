#pragma once
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <vector>

class VulkanDevice;

class VulkanSync
{
public:
	// We now need the image count to create the correct number of semaphores
	VulkanSync(const VulkanDevice& device, size_t maxFramesInFlight, size_t swapchainImageCount);
	~VulkanSync();

	// These per Per-Frame
	const vk::raii::Semaphore& getImageAvailableSemaphore(size_t frameIndex) const { return m_imageAvailableSemaphores[frameIndex]; }
	const vk::raii::Fence& getInFlightFence(size_t frameIndex) const { return m_inFlightFences[frameIndex]; }

	// This one becomes Per-Image!
	const vk::raii::Semaphore& getRenderFinishedSemaphore(size_t imageIndex) const { return m_renderFinishedSemaphores[imageIndex]; }

	// Helper to resize if swapchain changes
	void refresh(size_t swapchainImageCount);

private:
	const VulkanDevice& m_device;
	std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores; // [MAX_FRAMES]
	std::vector<vk::raii::Fence> m_inFlightFences;             // [MAX_FRAMES]

	std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores; // [IMAGE_COUNT]
};
