#include "VulkanApplication.hpp"
#include "Mesh.hpp"
#include "DebugOutput.hpp"
#include "VulkanCommand.hpp" // for MAX_FRAMES_IN_FLIGHT

VulkanApplication::VulkanApplication(const std::string& AppName, const std::string& DeviceName, uint32_t w, uint32_t h) :
	appName(AppName),
	glfwContext(),
	vulkanInstance(appName, glfwContext.getRequiredInstanceExtensions()),
	vulkanWindow(vulkanInstance, w, h, appName),
	vulkanDevice(vulkanInstance, &vulkanWindow.getSurface(), DeviceName),
	vulkanLoader(vulkanDevice),
	renderer(vulkanDevice, vulkanWindow)
{
	// Create Fences (Signaled so we don't wait on first frame)
	constexpr vk::FenceCreateInfo fenceInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };

	for (uint32_t i = 0; i < VulkanCommand::MAX_FRAMES_IN_FLIGHT; ++i)
		{ m_inFlightFences.emplace_back(vulkanDevice.device(), fenceInfo); }
	LOG_DEBUG("VulkanApplication instance created");
}

VulkanApplication::~VulkanApplication()
{
	// Wait for GPU to finish all work
	// This protects members from being destroyed while still in use
	vulkanDevice.device().waitIdle();
	LOG_DEBUG("VulkanApplication instance destroyed");
}

void VulkanApplication::fillMesh()
{
	// A simple unit cube (radius 0.5) with colors
	// Position (x,y,z,w) | Color (r,g,b,a)
	m_mesh.vertices = {
		// Front Face (Z = -0.5)
		Vertex(std::array<float, 8>{-0.5f, -0.5f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f}), // 0: black
		Vertex(std::array<float, 8>{ 0.5f, -0.5f, -0.5f, 1.0f,  1.0f, 0.0f, 0.0f, 1.0f}), // 1: red
		Vertex(std::array<float, 8>{ 0.5f,  0.5f, -0.5f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f}), // 2: yellow
		Vertex(std::array<float, 8>{-0.5f,  0.5f, -0.5f, 1.0f,  0.0f, 1.0f, 0.0f, 1.0f}), // 3: green
		// Back Face (Z = +0.5)
		Vertex(std::array<float, 8>{-0.5f, -0.5f,  0.5f, 1.0f,  0.0f, 0.0f, 1.0f, 1.0f}), // 4: blue
		Vertex(std::array<float, 8>{ 0.5f, -0.5f,  0.5f, 1.0f,  1.0f, 0.0f, 1.0f, 1.0f}), // 5: magenta
		Vertex(std::array<float, 8>{ 0.5f,  0.5f,  0.5f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f}), // 6: white
		Vertex(std::array<float, 8>{-0.5f,  0.5f,  0.5f, 1.0f,  0.0f, 1.0f, 1.0f, 1.0f})  // 7: cyan
	};

	m_mesh.indices = {
		0, 1, 2, 2, 3, 0, // Front
		1, 5, 6, 6, 2, 1, // Right
		5, 4, 7, 7, 6, 5, // Back
		4, 0, 3, 3, 7, 4, // Left
		3, 2, 6, 6, 7, 3, // Top
		4, 5, 1, 1, 0, 4  // Bottom
	};
}

int VulkanApplication::run()
{
	LOG_DEBUG("VulkanApplication started run()");

	// 1. Prepare Data
	fillMesh();
	vulkanLoader.uploadMesh(m_mesh);

	uint32_t currentFrame = 0;

	// 2. Loop
	while (!vulkanWindow.shouldClose())
	{
		vulkanWindow.pollEvents();
		vulkanWindow.updateFPS(appName);

		// --- 
		const auto time = vulkanWindow.getTime();
		const auto cos_time = cos(time);
		const auto sin_time = sin(time);
		
		const glm::mat4 model = {
			cos_time,   0.0f,      sin_time, 0.0f,
			0.0f,       1.0f,      0.0f,     0.0f,
			-sin_time,  0.0f,      cos_time, 0.0f,
			0.0f,       0.0f,      0.0f,     1.0f };
		// ---

		auto& fence = m_inFlightFences[currentFrame];
		
		if (vulkanDevice.device().waitForFences({*fence}, vk::True, UINT64_MAX) != vk::Result::eSuccess)
			{ throw std::runtime_error("Fence wait failed"); }

		// 1. Compute Step (Future)
		// ... compute(..., fence) ...

		// 2. Render Step (Optional)
		// Renderer now guarantees 'fence' is signaled even if it skips drawing.
		renderer.draw(m_mesh, currentFrame, *fence, {}, model);
		
		// 3. Flow Guaranteed: Always advance
		currentFrame = VulkanCommand::advanceFrame(currentFrame);
	}
	return EXIT_SUCCESS;
}
