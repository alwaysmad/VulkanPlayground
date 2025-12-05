// src/VulkanDevice.cpp
#include "VulkanDevice.hpp"
#include "DebugOutput.hpp"
#include <set>
#include <vector>

static constexpr std::array requiredDeviceExtensions = { 
	vk::KHRSwapchainExtensionName,
	// vk::KHRSpirv14ExtensionName, // Core in Vulkan 1.2
	// vk::KHRSynchronization2ExtensionName, // Core in Vulkan 1.3
	// vk::KHRCreateRenderpass2ExtensionName // Core in Vulkan 1.2
};

static constexpr float queuePriority = 1.0f;

VulkanDevice::VulkanDevice(const vk::raii::Instance& instance, const vk::raii::SurfaceKHR& surface, const std::string& deviceName) :
	m_physicalDevice(nullptr),
	m_device(nullptr),
	m_graphicsQueue(nullptr),
	m_presentQueue(nullptr),
	m_computeQueue(nullptr),
	graphicsQueueIndex(UINT32_MAX),
	presentQueueIndex(UINT32_MAX),
	computeQueueIndex(UINT32_MAX)
{
	////////////////////////////////////////////////////////////////////////////////
	// Find and pick physical device
	////////////////////////////////////////////////////////////////////////////////
	const auto devices = instance.enumeratePhysicalDevices();
	
	if (devices.empty()) { throw std::runtime_error("Failed to find any device with Vulkan support"); }

	if constexpr (enableValidationLayers)
	{
		LOG_DEBUG("Available Physical Devices (" << devices.size() << ") :");
		for (const auto& device : devices) { LOG_DEBUG("\t" << device.getProperties().deviceName); }
	}
	
	// Grab device with the provided name	
	for (const auto& device : devices)
		{ if ( device.getProperties().deviceName == deviceName ) m_physicalDevice = device; }

	// In debug build
	if constexpr (enableValidationLayers)
	{
		// If prefered device not found just grab the first one
		if (*m_physicalDevice == nullptr)
		{
			m_physicalDevice = devices.front();
			LOG_DEBUG("Could not find requested device: '" << deviceName << "'");
		}
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
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT >();
	
	if (!features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters)
		{ throw std::runtime_error("Selected device does not support shader draw parameters"); }
	if (!features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering)
		{ throw std::runtime_error("Selected device does not support dynamic Rendering feature"); }
	if (!features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState)
		{ throw std::runtime_error("Selected device does not support extended Dynamic State feature"); }
		
	const auto availableExtensions = m_physicalDevice.enumerateDeviceExtensionProperties();

	if constexpr (enableValidationLayers)
	{
		LOG_DEBUG("Available device extensions (" << availableExtensions.size() << ") :");
		for (const auto& i : availableExtensions) { LOG_DEBUG("\t" << i.extensionName); }

		LOG_DEBUG("Required device extensions (" << requiredDeviceExtensions.size() << ") :");
		for (const auto& i : requiredDeviceExtensions) { LOG_DEBUG("\t" << i); }
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
		for (uint32_t i = 0; i < queueFamilies.size(); ++i)
		{
			std::string flags;
			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics) flags += "G";
			else flags += "-";
			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) flags += "C";
			else flags += "-";
			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eTransfer) flags += "T";
			else flags += "-";
			if (m_physicalDevice.getSurfaceSupportKHR(i, *surface)) flags += "P";
			else flags += "-";
			// There is more, but we don't need them
			LOG_DEBUG("\t" << i << " : " << flags);
		}
	}

	// 1. Try to find a queue family that supports BOTH Graphics and Present
	for (uint32_t i = 0; i < queueFamilies.size(); ++i)
	{
		const bool supportsGraphics = !!(queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics);
		const bool supportsPresent  = m_physicalDevice.getSurfaceSupportKHR(i, *surface);
		
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

	if (graphicsQueueIndex == UINT32_MAX) { throw std::runtime_error("Failed to find graphics queue family"); }
	else { LOG_DEBUG("Selected " << graphicsQueueIndex << " as graphics queue family"); }
	if (presentQueueIndex == UINT32_MAX) { throw std::runtime_error("Failed to find queue family that can present to surface"); }
	else { LOG_DEBUG("Selected " << presentQueueIndex << " as present queue family"); }
	if (computeQueueIndex == UINT32_MAX) { throw std::runtime_error("Failed to find compute queue family"); }
	else { LOG_DEBUG("Selected " << computeQueueIndex << " as compute queue family"); }

	////////////////////////////////////////////////////////////////////////////////
	// Prepare Queue Creation Info
	////////////////////////////////////////////////////////////////////////////////
	// We use a set to ensure we only create ONE queue info per unique family.
	const std::set<uint32_t> uniqueQueueFamilies = { graphicsQueueIndex, presentQueueIndex, computeQueueIndex };

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
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
	> featureChain = {
		// Core Features 1.0 (Enable samplerAnisotropy if needed)
		{ .features = { .samplerAnisotropy = vk::True } },

		// Vulkan 1.1 Features
		{ .shaderDrawParameters = vk::True },

		// Vulkan 1.3 Features
		{ .dynamicRendering = vk::True },

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
	m_graphicsQueue = m_device.getQueue(graphicsQueueIndex, 0);
	m_presentQueue = m_device.getQueue(presentQueueIndex, 0);
	m_computeQueue = m_device.getQueue(computeQueueIndex, 0);

	LOG_DEBUG("Graphics, Present and Compute queues retrieved");

}
