#pragma once
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <vector>

// Forward declarations to avoid circular includes
class VulkanDevice;
class VulkanWindow;

class VulkanSwapchain
{
public:
	VulkanSwapchain(const VulkanDevice&, const VulkanWindow&);

	// Called when window is resized
	void recreate();

	const vk::raii::SwapchainKHR& get() const { return m_swapchain; }
	const vk::Format& getImageFormat() const { return m_imageFormat; }
	const vk::Extent2D& getExtent() const { return m_extent; }
	const std::vector<vk::raii::ImageView>& getImageViews() const { return m_imageViews; }

private:
	// We keep references to dependencies so we can recreate() later
	const VulkanDevice& m_device;
	const VulkanWindow& m_window;

	vk::raii::SwapchainKHR m_swapchain;

	// Swapchain resources
	std::vector<vk::Image> m_images; // Handles (owned by swapchain)
	std::vector<vk::raii::ImageView> m_imageViews; // Wrappers (owned by us)

	vk::Format m_imageFormat;
	vk::Extent2D m_extent;

	// Internal helpers
	void createSwapchain();
	void createImageViews();
};
