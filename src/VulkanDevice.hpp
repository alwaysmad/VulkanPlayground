// src/VulkanDevice.hpp
#pragma once
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class VulkanDevice
{
public:
	VulkanDevice(const vk::raii::Instance&, const vk::raii::SurfaceKHR&, const std::string&);

	inline const vk::raii::Device& device() const { return m_device; }
	inline const vk::raii::Queue& graphicsQueue() const { return m_graphicsQueue; }
	inline const vk::raii::Queue& presentQueue() const { return m_presentQueue; }
	inline const vk::raii::Queue& computeQueue() const { return m_computeQueue; }
	inline const vk::raii::PhysicalDevice& physicalDevice() const { return m_physicalDevice; }

	inline uint32_t getGraphicsQueueIndex() const { return graphicsQueueIndex; }
	inline uint32_t getPresentQueueIndex() const { return presentQueueIndex; }

private:
	vk::raii::PhysicalDevice m_physicalDevice;
	vk::raii::Device m_device;

	vk::raii::Queue m_graphicsQueue;
	vk::raii::Queue m_presentQueue;
	vk::raii::Queue m_computeQueue;

	uint32_t graphicsQueueIndex;
	uint32_t presentQueueIndex;
	uint32_t computeQueueIndex;
};
