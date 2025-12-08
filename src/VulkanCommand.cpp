#include "VulkanCommand.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanPipeline.hpp"
#include "DebugOutput.hpp"

VulkanCommand::VulkanCommand(const VulkanDevice& device, 
                             const VulkanSwapchain& swapchain, 
                             const VulkanPipeline& pipeline)
	: m_swapchain(swapchain),
	  m_pipeline(pipeline),
	  // 1. Create Pool
	  m_commandPool(
		  device.device(),
		  vk::CommandPoolCreateInfo{
			  .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			  .queueFamilyIndex = device.getGraphicsQueueIndex()
		  }
	  ),
	  // 2. Allocate Buffers directly into member
	  // vk::raii::CommandBuffers IS a vector, so we just construct it.
	  m_commandBuffers(
		  device.device(),
		  vk::CommandBufferAllocateInfo{
			  .commandPool = m_commandPool,
			  .level = vk::CommandBufferLevel::ePrimary,
			  .commandBufferCount = MAX_FRAMES_IN_FLIGHT
		  }
	  )
{
	LOG_DEBUG("Command Pool created and " << m_commandBuffers.size() << " buffers allocated");
	LOG_DEBUG("VulkanCommand created");
}

VulkanCommand::~VulkanCommand()
{
	// Destruction handled by RAII objects in m_commandBuffers and m_commandPool
	LOG_DEBUG("VulkanCommand destroyed");
}

void VulkanCommand::recordDraw (size_t bufferIndex, size_t imageIndex)
{
	const auto& cmd = m_commandBuffers[bufferIndex];
	const auto& swapchainImageView = m_swapchain.getImageViews()[imageIndex];
	const auto& swapchainImage = m_swapchain.getImages()[imageIndex];
	const auto& extent = m_swapchain.getExtent();
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
	const vk::DependencyInfo dependency_info_1 = {
		.dependencyFlags         = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers    = &preRenderBarrier
	};
	cmd.pipelineBarrier2(dependency_info_1);
	// -------------------------------------------------------------------------
	// Rendering Info (Dynamic Rendering)
	// -------------------------------------------------------------------------
	const vk::ClearValue clearColor {
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
	// Dynamic State: Viewport & Scissor
	const vk::Viewport viewport {
		.x = 0.0f, .y = 0.0f,
		.width = static_cast<float>(extent.width),
		.height = static_cast<float>(extent.height),
		.minDepth = 0.0f, .maxDepth = 1.0f
	};
	cmd.setViewport(0, viewport);

	const vk::Rect2D scissor {
		.offset = {0, 0},
		.extent = extent
	};
	cmd.setScissor(0, scissor);

	// Draw 3 vertices (Triangle)
	cmd.draw(3, 1, 0, 0);

	cmd.endRendering();
	// -------------------------------------------------------------------------
	// BARRIER 2: Color Attachment -> Present Src
	// -------------------------------------------------------------------------
	const vk::ImageMemoryBarrier2 postRenderBarrier {
		.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
		.dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
		.dstAccessMask = vk::AccessFlagBits2::eNone, // Layout transition visible to presentation engine automatically
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
	const vk::DependencyInfo dependency_info_2 = {
		.dependencyFlags         = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers    = &postRenderBarrier
	};
	cmd.pipelineBarrier2(dependency_info_2);
	cmd.end();
}
