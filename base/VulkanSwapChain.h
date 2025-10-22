/*
* Class wrapping access to the swap chain
* 
* A swap chain is a collection of framebuffers used for rendering and presentation to the windowing system
*
* Copyright (C) 2016-2025 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <stdlib.h>
#include <string>
#include <assert.h>
#include <stdio.h>
#include <vector>

#include <vulkan/vulkan.h>
#include "VulkanTools.h"

class VulkanSwapChain
{
private: 
	VkInstance instance{ VK_NULL_HANDLE };
	VkDevice device{ VK_NULL_HANDLE };
	VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
	VkSurfaceKHR surface{ VK_NULL_HANDLE };
public:
	VkFormat colorFormat{};
	VkColorSpaceKHR colorSpace{};
	VkSwapchainKHR swapChain{ VK_NULL_HANDLE };
	std::vector<VkImage> images{};
	std::vector<VkImageView> imageViews{};
	uint32_t queueNodeIndex{ UINT32_MAX };
	uint32_t imageCount{ 0 };

#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
	void initSurface(IDirectFB* dfb, IDirectFBSurface* window);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	void initSurface(wl_display* display, wl_surface* window);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	void initSurface(xcb_connection_t* connection, xcb_window_t window);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
	void initSurface(uint32_t width, uint32_t height);
#endif

	/* Set the Vulkan objects required for swapchain creation and management, must be called before swapchain creation */
	void setContext(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
	/**
	* Create the swapchain and get its images with given width and height
	* 
	* @param width Pointer to the width of the swapchain (may be adjusted to fit the requirements of the swapchain)
	* @param height Pointer to the height of the swapchain (may be adjusted to fit the requirements of the swapchain)
	* @param vsync (Optional, default = false) Can be used to force vsync-ed rendering (by using VK_PRESENT_MODE_FIFO_KHR as presentation mode)
	*/
	void create(uint32_t& width, uint32_t& height, bool vsync = false, bool fullscreen = false);
	/**
	* Acquires the next image in the swap chain
	* 
	* @param presentCompleteSemaphore (Optional) Semaphore that is signaled when the image is ready for use
	* @param imageIndex Pointer to the image index that will be increased if the next image could be acquired
	* 
	* @note The function will always wait until the next image has been acquired by setting timeout to UINT64_MAX
	* 
	* @return VkResult of the image acquisition
	*/
	VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t& imageIndex);
	/* Free all Vulkan resources acquired by the swapchain */
	void cleanup();
};
