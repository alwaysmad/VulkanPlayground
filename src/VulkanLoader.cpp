#include "VulkanLoader.hpp"
#include "DebugOutput.hpp"

VulkanLoader::VulkanLoader(const VulkanDevice& device)
	: m_device(device),
	  // Initialize Command Pool for TRANSFER queue
	  m_command(device, device.getTransferQueueIndex()) 
{
	LOG_DEBUG("VulkanLoader initialized");
}

VulkanLoader::~VulkanLoader()
{
	LOG_DEBUG("VulkanLoader destroyed");
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> 
VulkanLoader::uploadToDevice(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage)
{
	// 1. Staging (Host Visible)
	auto stagingResult = m_device.createBuffer(size, 
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	vk::raii::DeviceMemory sMem = std::move(stagingResult.second);
	vk::raii::Buffer       sBuf = std::move(stagingResult.first);

	// Map & Copy
	void* mapped = sMem.mapMemory(0, size);
	memcpy(mapped, data, size);
	sMem.unmapMemory();

	// 2. GPU Buffer (Device Local)
	auto [dBuf, dMem] = m_device.createBuffer(size, 
		vk::BufferUsageFlagBits::eTransferDst | usage, 
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	// 3. Record Copy Command
	const auto& cmd = m_command.getBuffer(0); // Use our Transfer command buffer
	
	cmd.reset();
	cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	vk::BufferCopy region{ .size = size };
	cmd.copyBuffer(*sBuf, *dBuf, region);
	cmd.end();

	// 4. Submit to TRANSFER Queue
	const vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*cmd };
	m_device.transferQueue().submit(submitInfo, nullptr);
	m_device.transferQueue().waitIdle(); // Blocking upload (Simple for now)

	return { std::move(dBuf), std::move(dMem) };
}

void VulkanLoader::uploadMesh(Mesh& mesh)
{
	if (mesh.vertices.empty()) return;

	// Vertices
	auto [vBuf, vMem] = uploadToDevice(
		mesh.vertices.data(), 
		sizeof(Vertex) * mesh.vertices.size(), 
		vk::BufferUsageFlagBits::eVertexBuffer
	);
	mesh.vertexBuffer = std::move(vBuf);
	mesh.vertexMemory = std::move(vMem);

	// Indices
	if (!mesh.indices.empty()) {
		auto [iBuf, iMem] = uploadToDevice(
			mesh.indices.data(), 
			sizeof(uint16_t) * mesh.indices.size(), 
			vk::BufferUsageFlagBits::eIndexBuffer
		);
		mesh.indexBuffer = std::move(iBuf);
		mesh.indexMemory = std::move(iMem);
	}

	LOG_DEBUG("Mesh uploaded via Transfer Queue");
}
