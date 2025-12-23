#include "Computer.hpp"
#include "DebugOutput.hpp"

Computer::Computer(const VulkanDevice& device) :
	m_device(device),
	// Use the Dedicated Compute Queue (faster on AMD/NVIDIA)
	m_command(device, device.getComputeQueueIndex()),
	m_pipeline(device)
	{ LOG_DEBUG("Computer initialized"); }

Computer::~Computer() { LOG_DEBUG("Computer destroyed"); }

void Computer::compute(vk::Fence fence, vk::Semaphore signalSemaphore)
{
	// 1. Reset Fence (CPU Sync)
	m_device.device().resetFences({fence});

	// 2. Record Commands
	const auto& cmd = m_command.getBuffer(0); // We only need 1 buffer for compute usually
	recordComputeCommands(cmd);

	// 3. Submit
	const vk::SubmitInfo submitInfo {
		.commandBufferCount = 1,
		.pCommandBuffers = &*cmd,
		.signalSemaphoreCount = signalSemaphore ? 1u : 0u,
		.pSignalSemaphores = signalSemaphore ? &signalSemaphore : nullptr
	};

	// Submit to the COMPUTE Queue (not Graphics)
	m_device.computeQueue().submit(submitInfo, fence);
}

void Computer::recordComputeCommands(const vk::raii::CommandBuffer& cmd)
{
	cmd.reset();
	cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

	// Bind Pipeline
	cmd.bindPipeline(vk::PipelineBindPoint::eCompute, *m_pipeline.getPipeline());

	// Bind Descriptors (Satellites + Earth)
	// [TODO: Binding logic will go here once we implement UBOs]
	// cmd.bindDescriptorSets(..., *m_pipeline.getLayout(), ...);

	// Dispatch
	// For now, hardcoded 1 workgroup to test
	cmd.dispatch(1, 1, 1);

	cmd.end();
}
