#pragma once
#include "VulkanDevice.hpp"
#include "VulkanCommand.hpp"
#include "ComputePipeline.hpp"

class Computer
{
public:
	explicit Computer(const VulkanDevice& device);
	~Computer();

	// The main function. 
	// - 'fence': Signaled when compute finishes (CPU Sync)
	// - 'signalSemaphore': Signaled when compute finishes (GPU Sync for Renderer)
	void compute(vk::Fence fence, vk::Semaphore signalSemaphore = nullptr);

	// We will add methods to register/update Satellites here later
	// void updateSatellites(...) 

private:
	const VulkanDevice& m_device;

	// Owns its own Queue (Compute Queue)
	VulkanCommand m_command;

	// Owns its own Pipeline
	ComputePipeline m_pipeline;

	void recordComputeCommands(const vk::raii::CommandBuffer& cmd);
};
