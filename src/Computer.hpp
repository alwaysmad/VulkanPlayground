#pragma once
#include "VulkanDevice.hpp"
#include "VulkanCommand.hpp"
#include "ComputePipeline.hpp"
#include "Satellite.hpp"
#include "Mesh.hpp"

class Computer
{
public:
	explicit Computer(const VulkanDevice& device);
	~Computer();

	// The main function. 
	// - 'fence': Signaled when compute finishes (CPU Sync)
	// - 'signalSemaphore': Signaled when compute finishes (GPU Sync for Renderer)
	void compute(vk::Fence fence, vk::Semaphore signalSemaphore = {});

	// Link the Data (Mesh + Satellites) to the Compute Pipeline
	void registerResources(const Mesh& earthMesh, const SatelliteNetwork& satNet);

private:
	const VulkanDevice& m_device;

	// Owns its own Queue (Compute Queue)
	VulkanCommand m_command;

	// Owns its own Pipeline
	ComputePipeline m_pipeline;

	// Descriptors
	vk::raii::DescriptorPool m_descriptorPool = nullptr;
	vk::raii::DescriptorSets m_descriptorSets = nullptr;
	
	// Store these to update push constants
	uint32_t pcData[2] = {0, 0};

	void createDescriptors();
	void recordComputeCommands(const vk::raii::CommandBuffer& cmd);
};
