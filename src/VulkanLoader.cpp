#include "VulkanLoader.hpp"

VulkanLoader::VulkanLoader(const VulkanDevice& device)
	: m_device(device),
	  m_command(device, device.getTransferQueueIndex()) 
	{ LOG_DEBUG("VulkanLoader initialized"); }

VulkanLoader::~VulkanLoader() { LOG_DEBUG("VulkanLoader destroyed"); }

std::pair<vk::raii::Buffer, TrackedDeviceMemory> 
VulkanLoader::uploadToDevice(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage)
{
	// 1. Staging
	// We rely on auto/structured binding to handle the types.
	auto stagingResult = m_device.createBuffer(size, 
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	// Move into local vars
	auto sMem = std::move(stagingResult.second); 
	auto sBuf = std::move(stagingResult.first);

	// Map (sMem acts like raii::DeviceMemory via operator->)
	void* mapped = sMem->mapMemory(0, size);
	memcpy(mapped, data, size);
	sMem->unmapMemory();

	// 2. GPU Buffer
	auto [dBuf, dMem] = m_device.createBuffer(size, 
		vk::BufferUsageFlagBits::eTransferDst | usage, 
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	// 3. Copy
	const auto& cmd = m_command.getBuffer(0);
	cmd.reset();
	cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	vk::BufferCopy region{ .size = size };
	cmd.copyBuffer(*sBuf, *dBuf, region);
	cmd.end();

	const vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*cmd };
	m_device.transferQueue().submit(submitInfo, nullptr);
	m_device.transferQueue().waitIdle();

	return { std::move(dBuf), std::move(dMem) };
}

void VulkanLoader::uploadMesh(Mesh& mesh)
{
	if (mesh.vertices.empty()) 
		{ throw std::runtime_error("Trying to upload empty mesh"); }
	auto [vBuf, vMem] = uploadToDevice(
		mesh.vertices.data(), 
		sizeof(Vertex) * mesh.vertices.size(),
		// Allow both Vertex Input AND Compute Storage
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer
	);	
	mesh.vertexBuffer = std::move(vBuf);
	mesh.vertexMemory = std::move(vMem);

	if (mesh.indices.empty()) 
		{ throw std::runtime_error("Trying to upload mesh with empty indices"); }
	auto [iBuf, iMem] = uploadToDevice(
		mesh.indices.data(), 
		sizeof(uint32_t) * mesh.indices.size(), 
		vk::BufferUsageFlagBits::eIndexBuffer
	);
	mesh.indexBuffer = std::move(iBuf);
	mesh.indexMemory = std::move(iMem);

	LOG_DEBUG("Mesh uploaded via Transfer Queue");
}
