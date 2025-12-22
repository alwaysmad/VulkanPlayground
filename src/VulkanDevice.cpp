// src/VulkanDevice.cpp
#include "VulkanInstance.hpp"
#include "VulkanDevice.hpp"

static constexpr std::array requiredDeviceExtensions = { 
	vk::KHRSwapchainExtensionName,
	// vk::KHRSpirv14ExtensionName, // Core in Vulkan 1.2
	// vk::KHRSynchronization2ExtensionName, // Core in Vulkan 1.3
	// vk::KHRCreateRenderpass2ExtensionName // Core in Vulkan 1.2
};

static constexpr float queuePriority = 1.0f;
static constexpr uint32_t ALLOCATION_WARNING_THRESHOLD = 4000;

VulkanDevice::VulkanDevice(const VulkanInstance& instance, const vk::raii::SurfaceKHR* surface, const std::string& deviceName) :
	m_physicalDevice(nullptr),
	m_device(nullptr),
	m_graphicsQueue(nullptr),
	m_presentQueue(nullptr),
	m_computeQueue(nullptr),
	m_transferQueue(nullptr),
	graphicsQueueIndex(UINT32_MAX),
	presentQueueIndex(UINT32_MAX),
	computeQueueIndex(UINT32_MAX),
	transferQueueIndex(UINT32_MAX)
{
	////////////////////////////////////////////////////////////////////////////////
	// Find and pick physical device
	////////////////////////////////////////////////////////////////////////////////
	const auto devices = instance.getInstance().enumeratePhysicalDevices();
	
	if (devices.empty()) { throw std::runtime_error("Failed to find any device with Vulkan support"); }

	if constexpr (enableValidationLayers)
	{
		LOG_DEBUG("Available Physical Devices (" << devices.size() << ") :");
		for ([[maybe_unused]] const auto& device : devices) { LOG_DEBUG("\t" << device.getProperties().deviceName); }
	}
	
	// Grab device with the provided name	
	for (const auto& device : devices)
		{ if ( device.getProperties().deviceName == deviceName ) m_physicalDevice = device; }

	// In debug build
	if constexpr (enableValidationLayers)
	{
		// If prefered device not found tell user
		if (*m_physicalDevice == nullptr)
		{
			m_physicalDevice = devices.front();
			LOG_DEBUG("Could not find requested device: '" << deviceName << "'");
		}
		// and just grab the first one
		LOG_DEBUG("Selected device: '" << m_physicalDevice.getProperties().deviceName << "'");
	}
	// In release build
	else
	{
		// If prefered device not found 
		if (*m_physicalDevice == nullptr)
		{
			// Yell at user
			std::cerr << "Could not find requested device: '" << deviceName << "')";
			// Output a list of available devices
			std::cerr << "Available Physical Devices (" << devices.size() << ") :";
			for (const auto& device : devices) { std::cerr << "\t" << device.getProperties().deviceName; }
			// Exit
			throw std::runtime_error("Could not find requested device: '" + deviceName + "'");
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// Check extentions and features and else
	////////////////////////////////////////////////////////////////////////////////	
	const auto props = m_physicalDevice.getProperties();
	if (props.apiVersion < vk::ApiVersion13)
		{ throw std::runtime_error("Selected device " + deviceName + " does not support Vulkan 1.3"); }

	if (props.deviceType != vk::PhysicalDeviceType::eDiscreteGpu)
	{
		std::cerr << DBG_COLOR_YELLOW
			<< "[WARNING] Selected device '" << deviceName
			<< "' is NOT a discrete GPU."
			<< DBG_COLOR_RESET << std::endl;
	}

	// check features
	const auto features = m_physicalDevice.template getFeatures2<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan12Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT >();
	
	if (!features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters)
		{ throw std::runtime_error("Selected device does not support shader draw parameters"); }
	if (!features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2)
		{ throw std::runtime_error("Selected device does not support synchronization2"); }
	if (!features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering)
		{ throw std::runtime_error("Selected device does not support dynamic Rendering feature"); }
	if (!features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState)
		{ throw std::runtime_error("Selected device does not support extended Dynamic State feature"); }
		
	const auto availableExtensions = m_physicalDevice.enumerateDeviceExtensionProperties();

	if constexpr (enableValidationLayers)
	{
		LOG_DEBUG("Available device extensions (" << availableExtensions.size() << ") :");
		for ([[maybe_unused]] const auto& i : availableExtensions) { LOG_DEBUG("\t" << i.extensionName); }

		LOG_DEBUG("Required device extensions (" << requiredDeviceExtensions.size() << ") :");
		for ([[maybe_unused]] const auto& i : requiredDeviceExtensions) { LOG_DEBUG("\t" << i); }
	}

	for (const auto& requiredDeviceExtension : requiredDeviceExtensions)
	{
		if (std::ranges::none_of(
					availableExtensions,
					[requiredDeviceExtension](auto const& extensionProperty)
					{ return strcmp(extensionProperty.extensionName, requiredDeviceExtension) == 0; }
				))
		{ throw std::runtime_error("Required device extension not supported: " + std::string(requiredDeviceExtension)); }
	}

	////////////////////////////////////////////////////////////////////////////////
	// Setting up queue families
	////////////////////////////////////////////////////////////////////////////////
	const auto queueFamilies = m_physicalDevice.getQueueFamilyProperties();

	if constexpr (enableValidationLayers)
	{
		LOG_DEBUG("Found (" << queueFamilies.size() << ") queue families");
		LOG_DEBUG("G - Graphics, C - Compute, T - Transfer, P - Present");
		for ([[maybe_unused]] uint32_t i = 0; i < queueFamilies.size(); ++i)
		{
			std::string flags;
			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) flags += "G";
			else flags += "-";
			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) flags += "C";
			else flags += "-";
			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer) flags += "T";
			else flags += "-";
			if (surface && m_physicalDevice.getSurfaceSupportKHR(i, **surface)) flags += "P";
			else flags += "-";
			// There is more, but we don't need them
			LOG_DEBUG("\t" << i << " : " << flags);
		}
	}

	if (surface)
	{
		// 1. Try to find a queue family that supports BOTH Graphics and Present
		for (uint32_t i = 0; i < queueFamilies.size(); ++i)
		{
			const bool supportsGraphics = !!(queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics);
			const bool supportsPresent  = m_physicalDevice.getSurfaceSupportKHR(i, **surface);
			
			if (supportsGraphics && supportsPresent)
			{
				graphicsQueueIndex = i;
				presentQueueIndex = i;
				break;
			}
		}

		// 2. If we didn't find a unified one, fallback to separate queues
		if (graphicsQueueIndex == UINT32_MAX)
		{
			for (uint32_t i = 0; i < queueFamilies.size(); ++i)
			{
				if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
					{ graphicsQueueIndex = i; break; }
			}
		}
		if (presentQueueIndex == UINT32_MAX)
		{
			for (uint32_t i = 0; i < queueFamilies.size(); ++i)
			{
				if (m_physicalDevice.getSurfaceSupportKHR(i, *surface))
					{ presentQueueIndex = i; break; }
			}
		}
	}
	// 3. Find Compute (Dedicated if possible, or reuse graphics)
	// Some vendors have a dedicated compute queue (Async Compute) which is faster.
	for (uint32_t i = 0; i < queueFamilies.size(); ++i)
	{
		if ((queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) && i != graphicsQueueIndex)
			{ computeQueueIndex = i; break; }
	}
	// Fallback: if no dedicated compute queue, use the graphics queue
	if (computeQueueIndex == UINT32_MAX)
		{ computeQueueIndex = graphicsQueueIndex; }

	// 4. Try to find a dedicated transfer queue (has Transfer bit, but NO Graphics/Compute bits)
	// This is common on NVIDIA/AMD cards and allows faster DMA copies.
	for (uint32_t i = 0; i < queueFamilies.size(); ++i)
	{
		const auto flags = queueFamilies[i].queueFlags;
		if ((flags & vk::QueueFlagBits::eTransfer) &&
			!(flags & vk::QueueFlagBits::eGraphics) &&
			!(flags & vk::QueueFlagBits::eCompute))
			{ transferQueueIndex = i; break; }
	}
	// Fallback: Use Compute queue (often supports transfer)
	if (transferQueueIndex == UINT32_MAX && computeQueueIndex != UINT32_MAX)
		transferQueueIndex = computeQueueIndex;
	// Fallback: Use Graphics queue
	if (transferQueueIndex == UINT32_MAX)
		transferQueueIndex = graphicsQueueIndex;
	
	if (surface)
	{
		if (graphicsQueueIndex == UINT32_MAX) { throw std::runtime_error("Failed to find graphics queue family"); }
		else { LOG_DEBUG("Selected " << graphicsQueueIndex << " as graphics queue family"); }
		if (presentQueueIndex == UINT32_MAX) { throw std::runtime_error("Failed to find queue family that can present to surface"); }
		else { LOG_DEBUG("Selected " << presentQueueIndex << " as present queue family"); }
	}
	if (computeQueueIndex == UINT32_MAX) { throw std::runtime_error("Failed to find compute queue family"); }
	else { LOG_DEBUG("Selected " << computeQueueIndex << " as compute queue family"); }
	if (transferQueueIndex == UINT32_MAX) { throw std::runtime_error("Failed to find transfer queue family"); }
	else { LOG_DEBUG("Selected " << transferQueueIndex << " as transfer queue family"); }

	////////////////////////////////////////////////////////////////////////////////
	// Prepare Queue Creation Info
	////////////////////////////////////////////////////////////////////////////////
	// We use a set to ensure we only create ONE queue info per unique family.
	const std::set<uint32_t> uniqueQueueFamilies = {
		(surface ? graphicsQueueIndex: computeQueueIndex),
		(surface ? presentQueueIndex : computeQueueIndex),
		computeQueueIndex,
		transferQueueIndex};

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

	for (const auto& queueFamily : uniqueQueueFamilies)
	{
		queueCreateInfos.emplace_back(
			vk::DeviceQueueCreateInfo{
				.queueFamilyIndex = queueFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority
			});
	}

	////////////////////////////////////////////////////////////////////////////////
	// Create Logical Device
	////////////////////////////////////////////////////////////////////////////////
	// Create a chain of feature structures
	const vk::StructureChain<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan12Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
	> featureChain = {
		// Core Features 1.0 (Enable samplerAnisotropy if needed)
		{ .features = { .samplerAnisotropy = vk::True } },
		// Vulkan 1.1 Features
		{
			.storagePushConstant16 = vk::True,
			.shaderDrawParameters = vk::True
		},
		// Vulkan 1.2 Features
		{
			.shaderFloat16 = vk::True,         // <--- NEW: Enable 'half' in shaders
			// .bufferDeviceAddress = vk::True // (Commonly enabled here too, but optional for now)
		},
		// Vulkan 1.3 Features
		{
			.synchronization2 = vk::True,
			.dynamicRendering = vk::True
		},
		// Extended Dynamic State
		{ .extendedDynamicState = vk::True }
	};

	const vk::DeviceCreateInfo deviceCreateInfo {
		.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
		.ppEnabledExtensionNames = requiredDeviceExtensions.data()
	};

	m_device = vk::raii::Device(m_physicalDevice, deviceCreateInfo);
	LOG_DEBUG("Logical Device created successfully");

	////////////////////////////////////////////////////////////////////////////////
	// Get Queue Handles
	////////////////////////////////////////////////////////////////////////////////
	// We request index 0 for both.
	// If they are the same family, we are sharing the same actual queue, which is fine.
	if (surface)
	{
		m_graphicsQueue = m_device.getQueue(graphicsQueueIndex, 0);
		m_presentQueue = m_device.getQueue(presentQueueIndex, 0);
	}
	m_computeQueue = m_device.getQueue(computeQueueIndex, 0);
	m_transferQueue = m_device.getQueue(transferQueueIndex, 0);

	LOG_DEBUG("Queues retrieved");
}

// Helper to find correct memory type (e.g., DeviceLocal vs HostVisible)
uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
{
	const auto memProperties = m_physicalDevice.getMemoryProperties();
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		// 1. Check if the bit is set in the filter
		// 2. Check if the memory type has ALL the requested properties
		if ( (typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{ return i; }
	}
	throw std::runtime_error("failed to find suitable memory type for buffer");
}

std::pair<vk::raii::Buffer, TrackedDeviceMemory> VulkanDevice::createBuffer(
		vk::DeviceSize size, 
		vk::BufferUsageFlags usage, 
		vk::MemoryPropertyFlags properties) const 
{
	// 1. Create Buffer
	const vk::BufferCreateInfo bufferInfo {
		.size = size,
		.usage = usage,
		.sharingMode = vk::SharingMode::eExclusive 
	};
	vk::raii::Buffer buffer(m_device, bufferInfo);

	// 2. Find Memory
	const auto memRequirements = buffer.getMemoryRequirements();
	const uint32_t memType = findMemoryType(memRequirements.memoryTypeBits, properties);

	// 3. Check Limits (Querying the Static Member)
	// We check BEFORE allocation to warn about what is about to happen
	if (TrackedDeviceMemory::allocationCount >= ALLOCATION_WARNING_THRESHOLD)
	{
		std::cerr << DBG_COLOR_YELLOW 
			<< "[WARNING] High memory allocation count: " 
			<< TrackedDeviceMemory::allocationCount
			<< " (Limit ~4096)" << DBG_COLOR_RESET << std::endl;
	}

	// 4. Allocate
	const vk::MemoryAllocateInfo allocInfo{
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = memType 
	};
	vk::raii::DeviceMemory memory(m_device, allocInfo);

	// 5. Bind
	buffer.bindMemory(*memory, 0);

	// 6. Return Wrapped
	return { std::move(buffer), TrackedDeviceMemory(std::move(memory)) };
}

std::pair<vk::raii::Image, TrackedDeviceMemory> VulkanDevice::createImage(
		const vk::ImageCreateInfo& createInfo,
		vk::MemoryPropertyFlags properties) const
{
	vk::raii::Image image(m_device, createInfo);

	const auto memRequirements = image.getMemoryRequirements();
	const uint32_t memType = findMemoryType(memRequirements.memoryTypeBits, properties);

	// We check BEFORE allocation to warn about what is about to happen
	if (TrackedDeviceMemory::allocationCount >= ALLOCATION_WARNING_THRESHOLD)
	{
		std::cerr << DBG_COLOR_YELLOW 
			<< "[WARNING] High memory allocation count: " 
			<< TrackedDeviceMemory::allocationCount
			<< " (Limit ~4096)" << DBG_COLOR_RESET << std::endl;
	}

	const vk::MemoryAllocateInfo allocInfo{
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = memType
	};
	vk::raii::DeviceMemory memory(m_device, allocInfo);

	image.bindMemory(*memory, 0);

	return { std::move(image), TrackedDeviceMemory(std::move(memory)) };
}

vk::Format VulkanDevice::findSupportedFormat(
		const std::vector<vk::Format>& candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features) const
{
	for (const vk::Format& format : candidates)
	{
		const vk::FormatProperties props = m_physicalDevice.getFormatProperties(format);

		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
			return format;
		else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
			return format;
	}
	throw std::runtime_error("failed to find supported format!");
}
