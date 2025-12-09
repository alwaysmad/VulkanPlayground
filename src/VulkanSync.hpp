#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <vector>
#include "VulkanDevice.hpp"

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
	inline void refresh(size_t swapchainImageCount)
	{
		m_renderFinishedSemaphores.clear();
		m_renderFinishedSemaphores.reserve(swapchainImageCount);
		constexpr vk::SemaphoreCreateInfo semaphoreInfo{};

		for (size_t i = 0; i < swapchainImageCount; ++i)
			{ m_renderFinishedSemaphores.emplace_back(m_device.device(), semaphoreInfo); }
	}

private:
	const VulkanDevice& m_device;
	std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores; // [MAX_FRAMES]
	std::vector<vk::raii::Fence> m_inFlightFences;             // [MAX_FRAMES]
	std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores; // [IMAGE_COUNT]
};
