#include "Renderer.hpp"
#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "DebugOutput.hpp"

Renderer::Renderer(const VulkanDevice& device, const VulkanWindow& window)
	: m_device(device), 
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
	size_t imageCount = m_swapchain.getImages().size();
	// Re-create semaphores if image count changed
	if (m_renderFinishedSemaphores.size() != imageCount)
	{
		m_renderFinishedSemaphores.clear();
		constexpr vk::SemaphoreCreateInfo semaphoreInfo{};
		for (size_t i = 0; i < imageCount; ++i)
			{ m_renderFinishedSemaphores.emplace_back(m_device.device(), semaphoreInfo); }
	}
}

Renderer::~Renderer() { LOG_DEBUG("Renderer destroyed"); }

void Renderer::recreateSwapchain()
{
	m_swapchain.recreate();
	remakeRenderFinishedSemaphores();
}

bool Renderer::draw(const Mesh& mesh, uint32_t currentFrame, const vk::Fence& fence, const vk::Semaphore* waitSemaphore)
{
	// 1. Acquire Image
	auto& imgSem = m_imageAvailableSemaphores[currentFrame];
	uint32_t imageIndex;
	try {
		auto result = m_device.device().acquireNextImage2KHR({
			.swapchain = *m_swapchain.getSwapchain(),
			.timeout = UINT64_MAX,
			.semaphore = *imgSem,
			.deviceMask = 1
		});
		imageIndex = result.second;
	} catch (const vk::OutOfDateKHRError&) {
		recreateSwapchain();
		return false; // Fence NOT reset, loop will retry immediately
	}

	// 2. Reset Fence (Only now that we know we are submitting)
	m_device.device().resetFences({fence});

	// 3. Record
	const auto& cmd = m_command.getBuffer(currentFrame);
	recordCommands(cmd, imageIndex, mesh);

	// 4. Submit
	// CHANGE: Use imageIndex instead of currentFrame for the signal semaphore
	auto& renderSem = m_renderFinishedSemaphores[imageIndex];

	std::vector<vk::Semaphore> waitSems;
	std::vector<vk::PipelineStageFlags> waitStages;

	// Internal Wait
	waitSems.push_back(*imgSem);
	waitStages.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);

	// External Wait
	if (waitSemaphore) {
		waitSems.push_back(*waitSemaphore);
		waitStages.push_back(vk::PipelineStageFlagBits::eVertexInput); 
	}

	const vk::SubmitInfo submitInfo {
		.waitSemaphoreCount = static_cast<uint32_t>(waitSems.size()),
		.pWaitSemaphores = waitSems.data(),
		.pWaitDstStageMask = waitStages.data(),
		.commandBufferCount = 1, .pCommandBuffers = &*cmd,
		.signalSemaphoreCount = 1, .pSignalSemaphores = &*renderSem // Using imageIndex
	};

	m_device.graphicsQueue().submit(submitInfo, fence);

	// 5. Present
	try {
		auto result = m_device.presentQueue().presentKHR({
			.waitSemaphoreCount = 1, .pWaitSemaphores = &*renderSem,
			.swapchainCount = 1, .pSwapchains = &*m_swapchain.getSwapchain(),
			.pImageIndices = &imageIndex
		});
		if (result == vk::Result::eSuboptimalKHR) { recreateSwapchain(); }
	} catch (const vk::OutOfDateKHRError&) {
		recreateSwapchain();
	}
	
	return true;
}

void Renderer::recordCommands(const vk::raii::CommandBuffer& cmd, uint32_t imageIndex, const Mesh& mesh)
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
		.subresourceRange = { .aspectMask = vk::ImageAspectFlagBits::eColor, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }
	};
	cmd.pipelineBarrier2({ .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &preRenderBarrier });

	// Rendering
	constexpr vk::ClearValue clearColor { .color = { std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} } };
	const vk::RenderingAttachmentInfo attachment {
		.imageView = swapchainImageView,
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor
	};
	
	cmd.beginRendering({ .renderArea = { .extent = extent }, .layerCount = 1, .colorAttachmentCount = 1, .pColorAttachments = &attachment });

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline.getPipeline());
	
	vk::Viewport vp { .width = (float)extent.width, .height = (float)extent.height, .maxDepth = 1.0f };
	cmd.setViewport(0, vp);
	vk::Rect2D scissor { .extent = extent };
	cmd.setScissor(0, scissor);
	
	cmd.pushConstants<std::array<float, 2>>(*m_pipeline.getLayout(), vk::ShaderStageFlagBits::eVertex, 0, m_swapchain.getScale());

	if (mesh.isUploaded())
	{
		cmd.bindVertexBuffers(0, {*mesh.vertexBuffer}, {0});
		if (mesh.indexBuffer != nullptr)
		{
			cmd.bindIndexBuffer(*mesh.indexBuffer, 0, vk::IndexType::eUint16);
			cmd.drawIndexed(mesh.indices.size(), 1, 0, 0, 0);
		}
		else
		{
			cmd.draw(mesh.vertices.size(), 1, 0, 0);
		}
	}

	cmd.endRendering();

	// Barrier: Color Attachment -> Present
	const vk::ImageMemoryBarrier2 postRenderBarrier {
		.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
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
