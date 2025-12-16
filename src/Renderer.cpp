#include "Renderer.hpp"
#include "VulkanCommand.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanPipeline.hpp"
#include "DebugOutput.hpp"

Renderer::Renderer(const VulkanDevice& device, const VulkanSwapchain& swapchain, 
                   const VulkanPipeline& pipeline, const VulkanCommand& command)
	: m_device(device), m_swapchain(swapchain), m_pipeline(pipeline), m_command(command)
	{ LOG_DEBUG("Renderer initialized"); }

Renderer::~Renderer()
	{ LOG_DEBUG("Renderer destroyed"); }

void Renderer::uploadMesh(Mesh& mesh)
{
	if (mesh.vertices.empty()) return;

	vk::DeviceSize vSize = sizeof(Vertex) * mesh.vertices.size();
	vk::DeviceSize iSize = sizeof(uint16_t) * mesh.indices.size();

	// --- 1. Create Staging Buffers (Host Visible) ---
	auto [sVBuf, sVMem] = m_device.createBuffer ( vSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent );

	// Map & Copy Vertices
	void* vData = sVMem.mapMemory(0, vSize);
	memcpy(vData, mesh.vertices.data(), vSize);
	sVMem.unmapMemory();

	// --- 2. Create GPU Buffers (Device Local) ---
	// Note usage: TransferDst + VertexBuffer
	auto [dVBuf, dVMem] = m_device.createBuffer ( vSize, 
			vk::BufferUsageFlagBits::eTransferDst |
			vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

	// --- 3. Execute Copy ---
	// We reuse the first command buffer since we aren't drawing yet
	const auto& cmd = m_command.getBuffer(0); 

	cmd.reset();
	cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

	const vk::BufferCopy copyRegion{ .size = vSize };
	cmd.copyBuffer(*sVBuf, *dVBuf, copyRegion);

	// (Repeat logic for Index Buffer here if needed)

	cmd.end();

	// --- 4. Submit & Wait ---
	const vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*cmd };
	m_device.graphicsQueue().submit(submitInfo, nullptr);
	m_device.graphicsQueue().waitIdle(); // Block until upload finishes

	// --- 5. Transfer ownership to Mesh ---
	mesh.vertexBuffer = std::move(dVBuf);
	mesh.vertexMemory = std::move(dVMem);
	// mesh.indexBuffer = ...

	LOG_DEBUG("Mesh uploaded to VRAM (" << mesh.vertices.size() << " vertices)");
}

void Renderer::drawFrame(const vk::raii::CommandBuffer& cmd, uint32_t imageIndex, const Mesh& mesh)
{
	const auto& swapchainImageView = m_swapchain.getImageViews()[imageIndex];
	const auto& swapchainImage = m_swapchain.getImages()[imageIndex];
	const auto extent = m_swapchain.getExtent();
	const auto scale = m_swapchain.getScale();

	// 1. Begin
	cmd.reset();
	cmd.begin({ .flags = {} });

	// -------------------------------------------------------------------------
	// BARRIER 1: Undefined -> Color Attachment
	// -------------------------------------------------------------------------
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
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0, .levelCount = 1,
			.baseArrayLayer = 0, .layerCount = 1
		}
	};
	const vk::DependencyInfo dependency_info_1 {
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers    = &preRenderBarrier
	};
	cmd.pipelineBarrier2(dependency_info_1);

	// -------------------------------------------------------------------------
	// Rendering Info
	// -------------------------------------------------------------------------
	constexpr vk::ClearValue clearColor {
		.color = { std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} }
	};
	const vk::RenderingAttachmentInfo colorAttachment {
		.imageView = swapchainImageView,
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor
	};
	const vk::RenderingInfo renderingInfo {
		.renderArea = { .offset = {0, 0}, .extent = extent },
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachment
	};
	cmd.beginRendering(renderingInfo);

	// -------------------------------------------------------------------------
	// Draw Commands
	// -------------------------------------------------------------------------
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline.getPipeline());

	// Dynamic State
	const vk::Viewport viewport {
		.x = 0.0f, .y = 0.0f,
		.width = static_cast<float>(extent.width),
		.height = static_cast<float>(extent.height),
		.minDepth = 0.0f, .maxDepth = 1.0f
	};
	cmd.setViewport(0, viewport);

	const vk::Rect2D scissor { .offset = {0, 0}, .extent = extent };
	cmd.setScissor(0, scissor);

	// Push Constants
	cmd.pushConstants<std::array<float, 2>> (
			*m_pipeline.getLayout(),
			vk::ShaderStageFlagBits::eVertex,
			0,
			scale
	);

	if (mesh.isUploaded())
	{
		cmd.bindVertexBuffers(0, {*mesh.vertexBuffer}, {0});
		// cmd.drawIndexed(mesh.indices.size(), 1, 0, 0, 0); // If you added index buffer
		cmd.draw(mesh.vertices.size(), 1, 0, 0); // Non-indexed for now
	}
	else
	{
		// Fallback or skip
		cmd.draw(3, 1, 0, 0);
	}

	cmd.endRendering();

	// -------------------------------------------------------------------------
	// BARRIER 2: Color Attachment -> Present Src
	// -------------------------------------------------------------------------
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
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0, .levelCount = 1,
			.baseArrayLayer = 0, .layerCount = 1
		}
	};
	const vk::DependencyInfo dependency_info_2 {
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers    = &postRenderBarrier
	};
	cmd.pipelineBarrier2(dependency_info_2);
	cmd.end();
}
