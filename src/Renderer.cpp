#include "Renderer.hpp"
#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "DebugOutput.hpp"
#include "Uniforms.hpp"

Renderer::Renderer(const VulkanDevice& device, const VulkanWindow& window) :
	m_device(device), 
	m_window(window),
	m_command(device, device.getGraphicsQueueIndex()),
	m_swapchain(device, window),
	m_pipeline(device, m_swapchain)
{
	// 1. Create Per-Frame Sync Objects (Image Available)
	constexpr vk::SemaphoreCreateInfo semaphoreInfo{};

	for (uint32_t i = 0; i < VulkanCommand::MAX_FRAMES_IN_FLIGHT; ++i)
		{ m_imageAvailableSemaphores.emplace_back(m_device.device(), semaphoreInfo); }

	// 2. Create Per-Image Sync Objects (Render Finished)
	// We need one per swapchain image to satisfy validation layers
	m_renderFinishedSemaphores.reserve(m_swapchain.getImages().size());
	remakeRenderFinishedSemaphores();
	LOG_DEBUG("Renderer initialized");
}

void Renderer::remakeRenderFinishedSemaphores()
{
	const uint32_t imageCount = m_swapchain.getImages().size();
	// Re-create semaphores if image count changed
	if (m_renderFinishedSemaphores.size() != imageCount)
	{
		m_renderFinishedSemaphores.clear();
		constexpr vk::SemaphoreCreateInfo semaphoreInfo{};
		for (uint32_t i = 0; i < imageCount; ++i)
			{ m_renderFinishedSemaphores.emplace_back(m_device.device(), semaphoreInfo); }
	}
}

Renderer::~Renderer() { LOG_DEBUG("Renderer destroyed"); }

void Renderer::recreateSwapchain()
{
	m_swapchain.recreate();
	remakeRenderFinishedSemaphores();
}

void Renderer::submitDummy(vk::Fence fence, vk::Semaphore waitSemaphore)
{
	// must reset the fence before submitting
	m_device.device().resetFences({fence});

	constexpr vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eAllCommands;
	
	const vk::SubmitInfo submitInfo {
		// If waitSemaphore exists, wait for it.
		.waitSemaphoreCount = waitSemaphore ? 1u : 0u,
		.pWaitSemaphores = waitSemaphore ? &waitSemaphore : nullptr,
		.pWaitDstStageMask = waitSemaphore ? &waitStage : nullptr,
		.commandBufferCount = 0, 
		.signalSemaphoreCount = 0
	};
	
	// Must signal 'fence' so the CPU knows this "frame" is done
	m_device.graphicsQueue().submit(submitInfo, fence);
}

void Renderer::draw(const Mesh& mesh, uint32_t currentFrame, vk::Fence fence, vk::Semaphore waitSemaphore, const glm::mat4& viewMatrix)
{
	// 0. Check for Minimization
	const auto extent = m_window.getExtent();
	if (extent.width == 0 || extent.height == 0)
		{ submitDummy(fence, waitSemaphore); return; }

	// 1. Acquire Image
	// Waits for 'imgSem' to be signaled when the presentation engine releases an image
	auto& imgSem = m_imageAvailableSemaphores[currentFrame];
	uint32_t imageIndex;
	try {
		const auto result = m_device.device().acquireNextImage2KHR({
			.swapchain = *m_swapchain.getSwapchain(),
			.timeout = UINT64_MAX,
			.semaphore = *imgSem,
			.deviceMask = 1
		});
		imageIndex = result.second;
	} catch (const vk::OutOfDateKHRError&)
		{ recreateSwapchain(); submitDummy(fence, waitSemaphore); return; }

	// 2. Reset Fence
	// We are about to submit work that will signal the fence.
	// We must reset it before submission.
	m_device.device().resetFences({fence});

	// 3. Record
	const auto& cmd = m_command.getBuffer(currentFrame);
	recordCommands(cmd, imageIndex, mesh, viewMatrix);

	// 4. Submit
	// Signals 'renderSem' when rendering finishes, so Present can start.
	auto& renderSem = m_renderFinishedSemaphores[imageIndex];

	// Define stages statically (Fixed mapping: Index 0 = Color, Index 1 = Vertex)
	constexpr std::array<vk::PipelineStageFlags, 2> waitStages = {
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eVertexInput
	};

	// If waitSemaphore is null, it sits harmlessly at index 1 because waitCount will be 1.
	const std::array<vk::Semaphore, 2> waitSems = { *imgSem, waitSemaphore };

	const vk::SubmitInfo submitInfo {
		.waitSemaphoreCount = waitSemaphore ? 2u : 1u,
		.pWaitSemaphores = waitSems.data(),
		.pWaitDstStageMask = waitStages.data(),
		.commandBufferCount = 1, .pCommandBuffers = &*cmd,
		.signalSemaphoreCount = 1, .pSignalSemaphores = &*renderSem
	};

	// COMMENT: Submit to queue.
	// - Waits on 'waitSems'
	// - Signals 'renderSem'
	// - Signals 'fence' when ALL work is done (for CPU sync)
	m_device.graphicsQueue().submit(submitInfo, fence);

	// 5. Present
	try {
		const auto result = m_device.presentQueue().presentKHR({
			// COMMENT: Wait for Rendering to finish before showing image
			.waitSemaphoreCount = 1, .pWaitSemaphores = &*renderSem,
			.swapchainCount = 1, .pSwapchains = &*m_swapchain.getSwapchain(),
			.pImageIndices = &imageIndex
		});
		if (result == vk::Result::eSuboptimalKHR) { recreateSwapchain(); }
	} catch (const vk::OutOfDateKHRError&)
		{ recreateSwapchain(); }
}

void Renderer::recordCommands(const vk::raii::CommandBuffer& cmd, uint32_t imageIndex, const Mesh& mesh, const glm::mat4& viewMatrix)
{
	const auto& swapchainImageView = m_swapchain.getImageViews()[imageIndex];
	const auto& swapchainImage = m_swapchain.getImages()[imageIndex];
	const auto extent = m_swapchain.getExtent();

	cmd.reset();
	cmd.begin({ .flags = {} });

	// Barrier: Undefined -> Color Attachment
	const vk::ImageMemoryBarrier2 preRenderBarrier {
		.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		.srcAccessMask = vk::AccessFlagBits2::eNone,
		.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
		.oldLayout = vk::ImageLayout::eUndefined,
		.newLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.image = swapchainImage,
		.subresourceRange = { .aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }
	};
	cmd.pipelineBarrier2({ .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &preRenderBarrier });

	// Rendering
	constexpr vk::ClearValue clearColor { .color = { backgroundColor } };
	const vk::RenderingAttachmentInfo attachment {
		.imageView = swapchainImageView,
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor
	};
	
	cmd.beginRendering({ .renderArea = { .extent = extent }, .layerCount = 1, .colorAttachmentCount = 1, .pColorAttachments = &attachment });

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline.getPipeline());
	
	const vk::Viewport vp { .width = (float)extent.width, .height = (float)extent.height, .maxDepth = 1.0f };
	cmd.setViewport(0, vp);
	const vk::Rect2D scissor { .extent = extent };
	cmd.setScissor(0, scissor);

	// --- Push camera and projection matrices ---
	CameraPushConstants constants;
	constants.view = viewMatrix;

	// Calculate Projection based on current window aspect ratio
	float aspect = (float)extent.width / (float)extent.height;
	constants.proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
	constants.proj[1][1] *= -1; // Fix Vulkan Y-flip

	cmd.pushConstants<CameraPushConstants>(*m_pipeline.getLayout(), vk::ShaderStageFlagBits::eVertex, 0, constants);
	// -------------------------

	// bind mesh to command and order to draw it	
	cmd.bindVertexBuffers(0, {*mesh.vertexBuffer}, {0});
	cmd.bindIndexBuffer(*mesh.indexBuffer, 0, vk::IndexType::eUint32);
	cmd.drawIndexed(mesh.indices.size(), 1, 0, 0, 0);

	cmd.endRendering();

	// Barrier: Color Attachment -> Present
	const vk::ImageMemoryBarrier2 postRenderBarrier {
		// We must wait for rendering to finish
		.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
		
		// BottomOfPipe is correct here: we just need to finish before the batch ends.
		.dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
		.dstAccessMask = vk::AccessFlagBits2::eNone, 
		
		.oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.newLayout = vk::ImageLayout::ePresentSrcKHR,
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.image = swapchainImage,
		.subresourceRange = { .aspectMask = vk::ImageAspectFlagBits::eColor, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }
	};
	cmd.pipelineBarrier2({ .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &postRenderBarrier });

	cmd.end();
}
