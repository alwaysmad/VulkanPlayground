#pragma once
#include "VulkanCommand.hpp"
#include "ComputePipeline.hpp"
#include "PushConstants.hpp"

class VulkanDevice;
class Mesh;
class SatelliteNetwork;

class Computer
{
public:
	explicit Computer(const VulkanDevice& device);
	~Computer();

	// The main function.
	// - 'fence': Signaled when compute finishes (CPU Sync). Pass vk::Fence{} if not needed.
	// - 'signalSemaphore': Signaled when compute finishes (GPU Sync). Pass vk::Semaphore{} if not needed.
	void compute (  uint32_t currentFrame,
			const glm::mat4& modelMatrix,
			float deltaTime,
			vk::Fence fence, 
			vk::Semaphore waitSemaphore,
			vk::Semaphore signalSemaphore = {} );
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
	
	// Store push constants
	ComputePushConstants m_pc;

	void recordComputeCommands(const vk::raii::CommandBuffer& cmd);

	// Constants for Dispatch
	// Must match [numthreads(256, 1, 1)] in shader
	static constexpr uint32_t blockSize = 256u;
	static constexpr uint32_t shift = 8u;
};
