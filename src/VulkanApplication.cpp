#include "VulkanApplication.hpp"
#include "VulkanCommand.hpp" // for MAX_FRAMES_IN_FLIGHT and advanceFrame()

VulkanApplication::VulkanApplication(const std::string& AppName, const std::string& DeviceName, uint32_t w, uint32_t h) :
	appName(AppName),
	glfwContext(),
	vulkanInstance(appName, glfwContext.getRequiredInstanceExtensions()),
	vulkanWindow(vulkanInstance, w, h, appName),
	vulkanDevice(vulkanInstance, &vulkanWindow.getSurface(), DeviceName),
	vulkanLoader(vulkanDevice),
	satelliteNetwork(vulkanDevice, 8), m_mesh(),
	computer(vulkanDevice),
	renderer(vulkanDevice, vulkanWindow)
{
	// Create Fences (Signaled so we don't wait on first frame)
	constexpr vk::FenceCreateInfo fenceInfo{ .flags = vk::FenceCreateFlagBits::eSignaled };

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{ m_inFlightFences.emplace_back(vulkanDevice.device(), fenceInfo); }

	// Create Semaphores
	constexpr vk::SemaphoreCreateInfo semInfo{};
	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{ m_computeFinishedSemaphores.emplace_back(vulkanDevice.device(), semInfo); }

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
		// Vertex(std::array<float, 8>{-0.5f, -0.5f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f}), // 0: black
		// Vertex(std::array<float, 8>{ 0.5f, -0.5f, -0.5f, 1.0f,  1.0f, 0.0f, 0.0f, 1.0f}), // 1: red
		// Vertex(std::array<float, 8>{ 0.5f,  0.5f, -0.5f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f}), // 2: yellow
		// Vertex(std::array<float, 8>{-0.5f,  0.5f, -0.5f, 1.0f,  0.0f, 1.0f, 0.0f, 1.0f}), // 3: green
		// Back Face (Z = +0.5)
		// Vertex(std::array<float, 8>{-0.5f, -0.5f,  0.5f, 1.0f,  0.0f, 0.0f, 1.0f, 1.0f}), // 4: blue
		// Vertex(std::array<float, 8>{ 0.5f, -0.5f,  0.5f, 1.0f,  1.0f, 0.0f, 1.0f, 1.0f}), // 5: magenta
		// Vertex(std::array<float, 8>{ 0.5f,  0.5f,  0.5f, 1.0f,  1.0f, 1.0f, 1.0f, 1.0f}), // 6: white
		// Vertex(std::array<float, 8>{-0.5f,  0.5f,  0.5f, 1.0f,  0.0f, 1.0f, 1.0f, 1.0f})  // 7: cyan
		//
		// Front Face (Z = -0.5)
		Vertex(std::array<float, 8>{-0.5f, -0.5f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f}), // 0: black
		Vertex(std::array<float, 8>{ 0.5f, -0.5f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f}), // 1: red
		Vertex(std::array<float, 8>{ 0.5f,  0.5f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f}), // 2: yellow
		Vertex(std::array<float, 8>{-0.5f,  0.5f, -0.5f, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f}), // 3: green
		// Back Face (Z = +0.5)
		Vertex(std::array<float, 8>{-0.5f, -0.5f,  0.5f, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f}), // 4: blue
		Vertex(std::array<float, 8>{ 0.5f, -0.5f,  0.5f, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f}), // 5: magenta
		Vertex(std::array<float, 8>{ 0.5f,  0.5f,  0.5f, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f}), // 6: white
		Vertex(std::array<float, 8>{-0.5f,  0.5f,  0.5f, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f})  // 7: cyan
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

// Helper for Cosine Palette in C++
// A simple wrapper to keep the main loop clean
static inline std::array<float, 4> getCosineColor(float t, float offset)
{
	// Simple Rainbow: r, g, b phase shifted
	const float r = 0.5f + 0.5f * std::cos(t + offset);
	const float g = 0.5f + 0.5f * std::cos(t + offset + 2.0f);
	const float b = 0.5f + 0.5f * std::cos(t + offset + 4.0f);
	return { r, g, b, 1.0f };
}

int VulkanApplication::run()
{
	LOG_DEBUG("VulkanApplication started run()");

	// 1. Prepare Data
	fillMesh();
	vulkanLoader.uploadMesh(m_mesh);
	satelliteNetwork.satellites.resize(8);
	
	// 2. Register Resources (Link Mesh + Satellites to Computer)
	computer.registerResources(m_mesh, satelliteNetwork);

	uint32_t currentFrame = 0;
	// 3. Loop
	while (!vulkanWindow.shouldClose())
	{
		vulkanWindow.pollEvents();
		vulkanWindow.updateFPS(appName);

		auto& fence = m_inFlightFences[currentFrame];
		auto& computeSem = m_computeFinishedSemaphores[currentFrame];

		// Wait for CPU to be ready
		if (vulkanDevice.device().waitForFences({*fence}, vk::True, UINT64_MAX) != vk::Result::eSuccess)
			{ throw std::runtime_error("Fence wait failed"); }

		const auto time = vulkanWindow.getTime();

		// --- Update Satellite Colors (Cosine) ---
		for (size_t i = 0; i < satelliteNetwork.satellites.size(); ++i)
		{
			// Give each index a different phase offset so they blink differently
			const float offset = float(i) * 0.8f;
			const auto col = getCosineColor(time * 2.0f, offset);

			// Copy to the 'data' field (which maps to 'color' in shader)
			std::memcpy(satelliteNetwork.satellites[i].data, col.data(), sizeof(col));
		}

		// Upload new data to UBO
        	satelliteNetwork.upload(currentFrame);

		// --- 
		constexpr auto rotSpeed = 0.2f;
		const auto cos_time = std::cos(time * rotSpeed);
		const auto sin_time = std::sin(time * rotSpeed);
		
		const glm::mat4 model = {
			cos_time,   0.0f,      sin_time, 0.0f,
			0.0f,       1.0f,      0.0f,     0.0f,
			-sin_time,  0.0f,      cos_time, 0.0f,
			0.0f,       0.0f,      0.0f,     1.0f };
		// ---

		// --- STEP 2: Compute Copy ---
		// Run the compute shader (Copies Satellite Color -> Vertex Color)
		// Pass nullptr for fence (we don't need CPU wait here)
		// Signal computeSem for the Graphics Queue
		computer.compute(currentFrame, satelliteNetwork, nullptr, *computeSem);

		// --- Render ---
		// Wait for computeSem before processing vertices
		renderer.draw(m_mesh, currentFrame, *fence, *computeSem, model);
		
		// 3. Flow Guaranteed: Always advance
		currentFrame = advanceFrame(currentFrame);
	}
	return EXIT_SUCCESS;
}
