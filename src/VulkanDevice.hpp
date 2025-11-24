// src/VulkanDevice.hpp
#pragma once
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class VulkanDevice
{
public:
	VulkanDevice(const vk::raii::Instance&, const vk::raii::SurfaceKHR&, const std::string&);

	const vk::raii::Device& device() const { return m_device; }
	const vk::raii::Queue& graphicsQueue() const { return m_graphicsQueue; }
	const vk::raii::Queue& presentQueue() const { return m_presentQueue; }
	const vk::raii::Queue& computeQueue() const { return m_computeQueue; }

private:
	vk::raii::PhysicalDevice m_physicalDevice;
	vk::raii::Device m_device;
	vk::raii::Queue m_graphicsQueue;
	vk::raii::Queue m_presentQueue;
	vk::raii::Queue m_computeQueue;
};
