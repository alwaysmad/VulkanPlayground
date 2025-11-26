#include "VulkanSwapchain.hpp"
#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "DebugOutput.hpp"
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

static inline vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats)
{
	for (const auto& availableFormat : formats)
	{
		// pick the good format
		if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
				availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			return availableFormat;
	}
	// or pick just the first one
	return formats[0];
}

static inline vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& presentModes)
{
	for (const auto& availablePresentMode : presentModes)
	{
		// Choose triple buffering if available
		if (availablePresentMode == vk::PresentModeKHR::eMailbox)
			return availablePresentMode;
	}
	// Otherwise pick V-sync as it's guaranteed to be available
	return vk::PresentModeKHR::eFifo;
}

static inline vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& caps, const VulkanWindow& window)
{
	if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return caps.currentExtent;
	
	vk::Extent2D actualExtent = window.getExtent();
	
	actualExtent.width = std::clamp(actualExtent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, caps.minImageExtent.height, caps.maxImageExtent.height);
	
	return actualExtent;
}

VulkanSwapchain::VulkanSwapchain(const VulkanDevice& device, const VulkanWindow& window) :
	m_device(device),
	m_window(window),
	m_swapchain(nullptr)
{
	createSwapchain();
	createImageViews();
}

void VulkanSwapchain::recreate()
{
	// Handle minimization (extent is 0,0)
	vk::Extent2D extent = m_window.getExtent();
	while (extent.width == 0 || extent.height == 0)
	{
		extent = m_window.getExtent();
		m_window.waitEvents();
	}

	m_device.device().waitIdle(); // Ensure GPU is done before destroying old swapchain

	// RAII handles destruction of old m_swapchain and m_imageViews here
	// when we overwrite them with new objects.
	createSwapchain();
	createImageViews();
	LOG_DEBUG("Swapchain recreated");
}

void VulkanSwapchain::createSwapchain()
{
	// 1. Query Support
	const auto& physicalDevice = m_device.physicalDevice();
	const auto& surface = m_window.getSurface();

	const auto caps = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	const auto formats = physicalDevice.getSurfaceFormatsKHR(surface);
	const auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

	// 2. Choose Settings
	auto surfaceFormat = chooseSwapSurfaceFormat(formats);
	auto presentMode = chooseSwapPresentMode(presentModes);
	auto extent = chooseSwapExtent(caps, m_window);

	////////////////////////////////////////////////////////////////////////////////
	// Logging details of swap chain support
	////////////////////////////////////////////////////////////////////////////////
	// const auto surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR( surface );
	// const std::vector<vk::SurfaceFormatKHR> availableFormats = m_physicalDevice.getSurfaceFormatsKHR( surface );
	//const std::vector<vk::PresentModeKHR> availablePresentModes = m_physicalDevice.getSurfacePresentModesKHR( surface );

	if constexpr (enableValidationLayers)
	{
		LOG_DEBUG("Surface Capabilities:");
		LOG_DEBUG("\tMin Image Count: " << caps.minImageCount);
		LOG_DEBUG("\tMax Image Count: " << caps.maxImageCount 
			<< (caps.maxImageCount == 0 ? " (unlimited)" : ""));
		
		LOG_DEBUG("\tCurrent Extent: " << caps.currentExtent.width 
			<< "x" << caps.currentExtent.height);
		
		LOG_DEBUG("\tMin Extent: " << caps.minImageExtent.width 
			<< "x" << caps.minImageExtent.height);
		
		LOG_DEBUG("\tMax Extent: " << caps.maxImageExtent.width 
			<< "x" << caps.maxImageExtent.height);

		LOG_DEBUG("\tMax Image Array Layers: " << caps.maxImageArrayLayers);
		LOG_DEBUG("\tSupported Transforms: " << to_string(caps.supportedTransforms));
		LOG_DEBUG("\tCurrent Transform: " << to_string(caps.currentTransform));
		LOG_DEBUG("\tSupported Composite Alpha: " << to_string(caps.supportedCompositeAlpha));
		LOG_DEBUG("\tSupported Usage Flags: " << to_string(caps.supportedUsageFlags));

		LOG_DEBUG("Available Surface Formats (" << formats.size() << "):");
		for (const auto& fmt : formats)
		{
			LOG_DEBUG("\tFormat: " << to_string(fmt.format) 
				<< " | ColorSpace: " << to_string(fmt.colorSpace));
		}

		LOG_DEBUG("Available Present Modes (" << presentModes.size() << "):");
		for (const auto& mode : presentModes)
		{
			LOG_DEBUG("\t" << to_string(mode));
		}
	}

	// 3. Image Count (Min + 1 is a good heuristic)
	// Aim for Triple Buffering
	uint32_t imageCount = std::clamp(3, caps.minImageCount, caps.maxImageCount);

    // 4. Sharing Mode (Graphics vs Present queues)
    // We need the indices we stored in VulkanDevice!
    uint32_t queueFamilyIndices[] = { m_device.graphicsQueueFamilyIndex, m_device.presentQueueFamilyIndex };
    
    vk::SharingMode sharingMode = vk::SharingMode::eExclusive;
    uint32_t indexCount = 0;
    uint32_t* pIndices = nullptr;

    if (m_device.graphicsQueueFamilyIndex != m_device.presentQueueFamilyIndex) {
        sharingMode = vk::SharingMode::eConcurrent;
        indexCount = 2;
        pIndices = queueFamilyIndices;
    }

    // 5. Create Info
    vk::SwapchainCreateInfoKHR createInfo {
        .surface = *surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = sharingMode,
        .queueFamilyIndexCount = indexCount,
        .pQueueFamilyIndices = pIndices,
        .preTransform = caps.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = vk::True,
        .oldSwapchain = nullptr // For now, simple recreation implies destroying old one first
    };

    // 6. Create
    // Note: We assign to the member variable, replacing any old one.
    m_swapchain = vk::raii::SwapchainKHR(m_device.device(), createInfo);
    
    // 7. Store attributes
    m_imageFormat = surfaceFormat.format;
    m_extent = extent;
    
    // 8. Get Images (Raw handles, not RAII)
    m_images = m_swapchain.getImages();
    
    LOG_DEBUG("Swapchain created (" << m_extent.width << "x" << m_extent.height << ")");
}

void VulkanSwapchain::createImageViews() {
    m_imageViews.clear(); // Clear old views if recreating
    m_imageViews.reserve(m_images.size());

    for (auto image : m_images) {
        vk::ImageViewCreateInfo createInfo {
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = m_imageFormat,
            .components = { 
                .r = vk::ComponentSwizzle::eIdentity,
                .g = vk::ComponentSwizzle::eIdentity,
                .b = vk::ComponentSwizzle::eIdentity,
                .a = vk::ComponentSwizzle::eIdentity 
            },
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        
        // Emplace back constructs the RAII object in the vector
        m_imageViews.emplace_back(m_device.device(), createInfo);
    }
}

