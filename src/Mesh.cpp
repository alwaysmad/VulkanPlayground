#include "Mesh.hpp"
#include "VulkanLoader.hpp"

// --- Vertex Implementation ---
static inline int16_t packSnorm(float f)
	{ return static_cast<int16_t>(std::round(std::clamp(f, -1.0f, 1.0f) * 32767.0f)); }

Vertex::Vertex(const std::array<float, 8>& data) : 
	posVar1{ packSnorm(data[0]), packSnorm(data[1]), packSnorm(data[2]), packSnorm(data[3]) },
	paramsVar2{ glm::packHalf1x16(data[4]), glm::packHalf1x16(data[5]), glm::packHalf1x16(data[6]), glm::packHalf1x16(data[7]) }
{}

// --- Mesh Implementation ---
Mesh::Mesh() { LOG_DEBUG("Mesh created"); }
Mesh::~Mesh() { LOG_DEBUG("Mesh destroyed"); }

void Mesh::upload(VulkanLoader& loader)
{
	if (vertices.empty()) 
		{ throw std::runtime_error("Trying to upload empty mesh"); }
	if (indices.empty()) 
		{ throw std::runtime_error("Trying to mesh with empty index buffer"); }
	
	// Create & Upload Vertex Buffer
	auto [vBuf, vMem] = loader.createBuffer(
		vertices.data(), 
		sizeof(Vertex) * vertices.size(),
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer
	);	
	vertexBuffer = std::move(vBuf);
	vertexMemory = std::move(vMem);

	// Create & Upload Index Buffer
	auto [iBuf, iMem] = loader.createBuffer(
		indices.data(), 
		sizeof(uint32_t) * indices.size(), 
		vk::BufferUsageFlagBits::eIndexBuffer
	);
	indexBuffer = std::move(iBuf);
	indexMemory = std::move(iMem);

	LOG_DEBUG("Mesh uploaded self via Loader");
}

void Mesh::download(VulkanLoader& loader)
{
	if (!*vertexBuffer)
		{ throw std::runtime_error("Trying download mesh from empty buffer"); }

	// Download Vertices
	// Note: We assume 'vertices' vector is already sized correctly or we resize it.
	// For now, assume size matches what's on GPU.
	loader.downloadBuffer(vertexBuffer, vertices.data(), sizeof(Vertex) * vertices.size());
	
	LOG_DEBUG("Mesh downloaded self via Loader");
}
