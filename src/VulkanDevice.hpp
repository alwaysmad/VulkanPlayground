// src/VulkanDevice.hpp
#pragma once
#include <vulkan/vulkan_raii.hpp>

class VulkanInstance;
class VulkanWindow;

class VulkanDevice
{
public:
	VulkanDevice(const VulkanInstance&, const VulkanWindow&, const std::string&);

	inline const vk::raii::Device& device() const { return m_device; }
	
	// Queues
	inline const vk::raii::Queue& graphicsQueue() const { return m_graphicsQueue; }
	inline const vk::raii::Queue& presentQueue() const { return m_presentQueue; }
	inline const vk::raii::Queue& computeQueue() const { return m_computeQueue; }
	inline const vk::raii::Queue& transferQueue() const { return m_transferQueue; }

	inline const vk::raii::PhysicalDevice& physicalDevice() const { return m_physicalDevice; }

	inline uint32_t getGraphicsQueueIndex() const { return graphicsQueueIndex; }
	inline uint32_t getPresentQueueIndex() const { return presentQueueIndex; }
	inline uint32_t getComputeQueueIndex() const { return computeQueueIndex; }
	inline uint32_t getTransferQueueIndex() const { return transferQueueIndex; }

	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer (
			vk::DeviceSize size,
			vk::BufferUsageFlags usage,
			vk::MemoryPropertyFlags properties ) const;

private:
	vk::raii::PhysicalDevice m_physicalDevice;
	vk::raii::Device m_device;

	vk::raii::Queue m_graphicsQueue;
	vk::raii::Queue m_presentQueue;
	vk::raii::Queue m_computeQueue;
	vk::raii::Queue m_transferQueue;

	uint32_t graphicsQueueIndex;
	uint32_t presentQueueIndex;
	uint32_t computeQueueIndex;
	uint32_t transferQueueIndex;

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;
};
