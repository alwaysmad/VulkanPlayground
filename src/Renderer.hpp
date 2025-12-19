#pragma once
#include "VulkanSwapchain.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanCommand.hpp" 
#include "Mesh.hpp"

class VulkanDevice;
class VulkanWindow;

class Renderer
{
public:
	Renderer(const VulkanDevice& device, const VulkanWindow& window);
	~Renderer();

	// - Returns void. The App controls the frame loop.
	// - Guarantees 'fence' is signaled, even if drawing fails/skips.
	//
	// SYNC OBJECTS:
	// - fence (Mandatory): 
	//     Signaled by GPU when this frame's submission completes. 
	//     Used by CPU (App) to wait before re-using resources (like CommandBuffers).
	//
	// - waitSemaphore (Optional): 
	//     If provided (not null), the Graphics Queue will wait for this semaphore 
	//     (at Vertex Input stage) before processing. Used to sync Compute -> Graphics.
	void draw(const Mesh& mesh,
		  uint32_t currentFrame,
		  vk::Fence fence,
		  vk::Semaphore waitSemaphore = {},
		  const glm::mat4& viewMatrix = defaultView);

private:

	// Default View: Eye(0, 1.5, 3), Center(0,0,0), Up(0,1,0)
	// Precomputed values:
	// cos(theta) = 3.0 / sqrt(1.5^2 + 3.0^2) = 0.894427
	// sin(theta) = 1.5 / sqrt(1.5^2 + 3.0^2) = 0.447214
	// Translation Z = -magnitude = -3.354102
	static constexpr glm::mat4 defaultView = {
		1.0f,       0.0f,       0.0f,       0.0f, // Col 0 (Right)
		0.0f,       0.894427f,  0.447214f,  0.0f, // Col 1 (Up-ish)
		0.0f,      -0.447214f,  0.894427f,  0.0f, // Col 2 (Forward-ish)
		0.0f,       0.0f,      -3.354102f,  1.0f  // Col 3 (Translation)
	};

	const VulkanDevice& m_device;
	const VulkanWindow& m_window;

	// Owned Resources
	VulkanCommand   m_command;
	VulkanSwapchain m_swapchain;
	VulkanPipeline  m_pipeline;

	// Sync
	// Fixed size [MAX_FRAMES_IN_FLIGHT]
	std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores;
	// This one must match Swapchain Image Count
	std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
	void remakeRenderFinishedSemaphores();
	
	void recordCommands(const vk::raii::CommandBuffer& cmd, uint32_t imageIndex, const Mesh& mesh, const glm::mat4& viewMatrix = defaultView);
	void recreateSwapchain();
	void submitDummy(vk::Fence fence, vk::Semaphore waitSemaphore);

	static constexpr std::array<float, 4> backgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};

	// Depth Resources
	vk::raii::Image     m_depthImage = nullptr;
	TrackedDeviceMemory m_depthMemory;
	vk::raii::ImageView m_depthView = nullptr;
	vk::Format          m_depthFormat;

	void createDepthBuffer();
};
