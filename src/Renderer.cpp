#include "Renderer.hpp"
#include "VulkanCommand.hpp"
#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "DebugOutput.hpp"

Renderer::Renderer(const VulkanDevice& device, const VulkanWindow& window, const VulkanCommand& command)
	: m_device(device), 
	  m_command(command),
	  m_window(window),
	  m_swapchain(device, window),
	  m_pipeline(device, m_swapchain),
	  m_sync(device, VulkanCommand::MAX_FRAMES_IN_FLIGHT, m_swapchain.getImages().size())
{
	LOG_DEBUG("Renderer initialized");
}

Renderer::~Renderer()
{
	m_device.device().waitIdle();
	LOG_DEBUG("Renderer destroyed");
}

void Renderer::recreateSwapchain()
{
	m_swapchain.recreate();
	m_sync.refresh(m_swapchain.getImages().size());
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> 
Renderer::uploadToDevice(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage)
{
	// --- 1. Staging (CPU) ---
	// FIX: Explicitly declare variables to ensure 'sBuf' (Buffer) is destroyed BEFORE 'sMem' (Memory)
	auto stagingResult = m_device.createBuffer(size, 
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	
	vk::raii::DeviceMemory sMem = std::move(stagingResult.second); // Memory declared 1st (dies 2nd)
	vk::raii::Buffer       sBuf = std::move(stagingResult.first);  // Buffer declared 2nd (dies 1st)

	// 2. Copy to Staging
	void* mapped = sMem.mapMemory(0, size);
	memcpy(mapped, data, size);
	sMem.unmapMemory();

	// --- 3. Device (GPU) ---
	auto [dBuf, dMem] = m_device.createBuffer(size, 
		vk::BufferUsageFlagBits::eTransferDst | usage, 
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	// 4. Copy Staging -> Device
	const auto& cmd = m_command.getBuffer(0);
	
	cmd.reset();
	cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	vk::BufferCopy region{ .size = size };
	cmd.copyBuffer(*sBuf, *dBuf, region);
	cmd.end();

	// 5. Submit & Wait
	const vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*cmd };
	m_device.graphicsQueue().submit(submitInfo, nullptr);
	m_device.graphicsQueue().waitIdle();

	return { std::move(dBuf), std::move(dMem) };
}

void Renderer::uploadMesh(Mesh& mesh)
{
	if (mesh.vertices.empty()) return;

	// 1. Upload Vertices
	auto [vBuf, vMem] = uploadToDevice(
		mesh.vertices.data(), 
		sizeof(Vertex) * mesh.vertices.size(), 
		vk::BufferUsageFlagBits::eVertexBuffer
	);
	// Move into Mesh (Correct order is handled by Mesh members now)
	mesh.vertexBuffer = std::move(vBuf);
	mesh.vertexMemory = std::move(vMem);

	// 2. Upload Indices
	if (!mesh.indices.empty()) {
		auto [iBuf, iMem] = uploadToDevice(
			mesh.indices.data(), 
			sizeof(uint16_t) * mesh.indices.size(), 
			vk::BufferUsageFlagBits::eIndexBuffer
		);
		mesh.indexBuffer = std::move(iBuf);
		mesh.indexMemory = std::move(iMem);
	}

	LOG_DEBUG("Mesh uploaded: " << mesh.vertices.size() << " verts, " << mesh.indices.size() << " indices");
}

void Renderer::draw(const Mesh& mesh)
{
	// 1. Wait
	auto& fence = m_sync.getInFlightFence(m_currentFrame);
	(void)m_device.device().waitForFences({*fence}, vk::True, UINT64_MAX);

	// 2. Acquire
	auto& imgSem = m_sync.getImageAvailableSemaphore(m_currentFrame);
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
		return;
	}

	m_device.device().resetFences({*fence});

	// 3. Record
	const auto& cmd = m_command.getBuffer(m_currentFrame);
	recordCommands(cmd, imageIndex, mesh);

	// 4. Submit
	auto& renderSem = m_sync.getRenderFinishedSemaphore(imageIndex);
	vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	
	const vk::SubmitInfo submitInfo {
		.waitSemaphoreCount = 1, .pWaitSemaphores = &*imgSem, .pWaitDstStageMask = &waitStage,
		.commandBufferCount = 1, .pCommandBuffers = &*cmd,
		.signalSemaphoreCount = 1, .pSignalSemaphores = &*renderSem
	};
	m_device.graphicsQueue().submit(submitInfo, *fence);

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

	m_currentFrame = VulkanCommand::advanceFrame(m_currentFrame);
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
