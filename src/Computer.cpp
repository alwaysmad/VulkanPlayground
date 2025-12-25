#include "Computer.hpp"

Computer::Computer(const VulkanDevice& device) :
	m_device(device),
	// Use the Dedicated Compute Queue (faster on AMD/NVIDIA)
	m_command(device, device.getComputeQueueIndex()),
	m_pipeline(device)
{
	createDescriptors();
	LOG_DEBUG("Computer initialized");
}

Computer::~Computer() { LOG_DEBUG("Computer destroyed"); }

void Computer::compute(vk::Fence fence, vk::Semaphore signalSemaphore)
{
	// 1. Reset Fence (CPU Sync)
	// Only reset/use fence if one is actually provided (in Headless mode)
	if (fence) { m_device.device().resetFences({fence}); }

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

	// Submit to the compute queue
	m_device.computeQueue().submit(submitInfo, fence);
}
	
void Computer::createDescriptors()
{
	// 1. Create Pool
	const std::array<vk::DescriptorPoolSize, 2> poolSizes =
	{{
		{ vk::DescriptorType::eUniformBuffer, 1 },
		{ vk::DescriptorType::eStorageBuffer, 1 }
	}};

	const vk::DescriptorPoolCreateInfo poolInfo {
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = 1,
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.data()
	};
	m_descriptorPool = vk::raii::DescriptorPool(m_device.device(), poolInfo);

	// 2. Allocate Set (Empty for now)
	const vk::DescriptorSetAllocateInfo allocInfo {
		.descriptorPool = *m_descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &*m_pipeline.getDescriptorSetLayout()
	};
	m_descriptorSets = vk::raii::DescriptorSets(m_device.device(), allocInfo);
}

void Computer::registerResources(const Mesh& earthMesh, const SatelliteNetwork& satNet)
{
	// 1. Update stored counts for Push Constants
	m_vertexCount = static_cast<uint32_t>(earthMesh.vertices.size());
	m_satelliteCount = static_cast<uint32_t>(satNet.satellites.size());

	// 2. Update Descriptors
	// Binding 0: Satellites (UBO)
	vk::DescriptorBufferInfo uboInfo {
		.buffer = *satNet.getBuffer(),
		.offset = 0,
		.range = satNet.getSize()
	};

	// Binding 1: Earth (SSBO)
	vk::DescriptorBufferInfo ssboInfo {
		.buffer = *earthMesh.vertexBuffer,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};

	const std::array<vk::WriteDescriptorSet, 2> descriptorWrites =
	{{
		{
			.dstSet = *m_descriptorSets[0],
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.pBufferInfo = &uboInfo
		},
		{
			.dstSet = *m_descriptorSets[0],
			.dstBinding = 1,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &ssboInfo
		}
	}};

	m_device.device().updateDescriptorSets(descriptorWrites, nullptr);
	LOG_DEBUG("Computer resources registered");
}

void Computer::recordComputeCommands(const vk::raii::CommandBuffer& cmd)
{
	cmd.reset();
	cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

	cmd.bindPipeline(vk::PipelineBindPoint::eCompute, *m_pipeline.getPipeline());

	// Bind the set we updated in registerResources
	cmd.bindDescriptorSets(
			vk::PipelineBindPoint::eCompute,
			*m_pipeline.getLayout(),
			0,
			{*m_descriptorSets[0]},
			nullptr
	);

	// --- PUSH CONSTANTS ---
	const uint32_t pcData[2] = { m_satelliteCount, m_vertexCount };
	cmd.pushConstants<uint32_t> (
			*m_pipeline.getLayout(),
			vk::ShaderStageFlagBits::eCompute,
			0,
			pcData
	);

	// Dispatch 1 thread per vertex (block size 256)
	uint32_t groupCount = (m_vertexCount + 255) / 256;
	cmd.dispatch(groupCount, 1, 1);

	cmd.end();
}
