#include "VulkanSync.hpp"
#include "DebugOutput.hpp"

VulkanSync::VulkanSync(const VulkanDevice& device, size_t maxFramesInFlight, size_t swapchainImageCount) 
	: m_device(device)
{
	constexpr vk::SemaphoreCreateInfo semaphoreInfo{};
	constexpr vk::FenceCreateInfo fenceInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };

	// Per-Frame Objects
	m_imageAvailableSemaphores.reserve(maxFramesInFlight);
	m_inFlightFences.reserve(maxFramesInFlight);
	for (size_t i = 0; i < maxFramesInFlight; ++i)
	{
		m_imageAvailableSemaphores.emplace_back(m_device.device(), semaphoreInfo);
		m_inFlightFences.emplace_back(m_device.device(), fenceInfo);
	}

	// Per-Image Objects
	refresh(swapchainImageCount);

	LOG_DEBUG("Synchronization objects created");
}

VulkanSync::~VulkanSync()
{
	LOG_DEBUG("Synchronization objects destroyed");
}
