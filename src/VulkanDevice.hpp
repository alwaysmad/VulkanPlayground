// src/VulkanDevice.hpp
#pragma once

// Forward declaration
class VulkanDevice;

// --- The Proxy Class ---
class TrackedDeviceMemory
{
public:
	inline static std::atomic<uint32_t> allocationCount { 0 };

	// Constructors
	TrackedDeviceMemory() = default; 
	
	explicit TrackedDeviceMemory(vk::raii::DeviceMemory&& mem) 
		: m_memory(std::move(mem)) 
		{ if (*m_memory) allocationCount++; }

	~TrackedDeviceMemory() { if (*m_memory) allocationCount--; }

	TrackedDeviceMemory(TrackedDeviceMemory&& other) noexcept 
		: m_memory(std::move(other.m_memory)) {}

	TrackedDeviceMemory& operator= (TrackedDeviceMemory&& other) noexcept {
		if (this != &other)
		{
			if (*m_memory) { allocationCount--; }
			m_memory = std::move(other.m_memory);
		}
		return *this;
	}

	TrackedDeviceMemory(const TrackedDeviceMemory&) = delete;
	TrackedDeviceMemory& operator=(const TrackedDeviceMemory&) = delete;

	// --- Transparency Operators  ---

	// 1. Dereference (*mem) returns the Underlying Handle (vk::DeviceMemory)
	// This allows usage in Vulkan API calls: buffer.bindMemory(*mem, 0);
	const vk::DeviceMemory& operator*() const { return *m_memory; }

	// 2. Arrow (mem->) returns the RAII Wrapper (vk::raii::DeviceMemory)
	// This allows usage of RAII methods: mem->mapMemory(...);
	vk::raii::DeviceMemory* operator->() { return &m_memory; }
	const vk::raii::DeviceMemory* operator->() const { return &m_memory; }

	// Accessor
	const vk::raii::DeviceMemory& get() const { return m_memory; }

private:
	vk::raii::DeviceMemory m_memory = nullptr;
};

// --- VulkanDevice ---
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

	[[nodiscard]] std::pair<vk::raii::Buffer, TrackedDeviceMemory> createBuffer (
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
