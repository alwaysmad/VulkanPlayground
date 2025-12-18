#include "VulkanPipeline.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "DebugOutput.hpp"
#include "Vertex.hpp"
#include "Uniforms.hpp"

#include "triangle.hpp" // The generated header

static constexpr auto  pushConstantSize = sizeof(CameraPushConstants);

[[nodiscard]] inline static vk::raii::ShaderModule createShaderModule(
		const vk::raii::Device& device,
		const unsigned char* data,
		size_t len)
{
	const vk::ShaderModuleCreateInfo createInfo {
		.codeSize = len,
		.pCode = reinterpret_cast<const uint32_t*>(data)
	};
	return vk::raii::ShaderModule(device, createInfo);
} 

VulkanPipeline::VulkanPipeline(
		const VulkanDevice& device,
		const VulkanSwapchain& swapchain) :
	m_device(device),
	m_pipelineLayout(nullptr),
	m_pipeline(nullptr)
{
	// 1. Load Shaders
	const vk::raii::ShaderModule shaderModule = createShaderModule(m_device.device(), triangle::code, triangle::size);
	// 2. Shader Stages
	const vk::PipelineShaderStageCreateInfo shaderStages[] = {
		{
			.stage = vk::ShaderStageFlagBits::eVertex,
			.module = shaderModule,
			.pName = "vertMain" // Entry point from Slang shader
		},
		{
			.stage = vk::ShaderStageFlagBits::eFragment,
			.module = shaderModule,
			.pName = "fragMain" // Entry point from Slang shader
		}
	};
	// 3. Vertex input
	constexpr vk::PipelineVertexInputStateCreateInfo vertexInputInfo {
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &VertexTraits::bindingDescription,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(VertexTraits::attributeDescriptions.size()),
		.pVertexAttributeDescriptions = VertexTraits::attributeDescriptions.data()
	};
	// 4. Input assembly
	constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssembly {
		.topology = vk::PrimitiveTopology::eTriangleList,
		.primitiveRestartEnable = vk::False
	};
	// 5. Viewport
	constexpr vk::PipelineViewportStateCreateInfo viewportState {
		.viewportCount = 1,
		.scissorCount = 1
	};
	// 6. Rasterizer
	constexpr vk::PipelineRasterizationStateCreateInfo rasterizer {
		.depthClampEnable = vk::False,
		.rasterizerDiscardEnable = vk::False,
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eClockwise, // Slang/Vulkan standard
		.depthBiasEnable = vk::False,
		.depthBiasSlopeFactor = 1.0f,
		.lineWidth = 1.0f
	};
	// 7. Multisampling
	constexpr vk::PipelineMultisampleStateCreateInfo multisampling {
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = vk::False
	};
	// 8. Color blending
	static constexpr vk::PipelineColorBlendAttachmentState colorBlendAttachment {
		.blendEnable = vk::False,
		.colorWriteMask
			= vk::ColorComponentFlagBits::eR
			| vk::ColorComponentFlagBits::eG
			| vk::ColorComponentFlagBits::eB
			| vk::ColorComponentFlagBits::eA
	};

	constexpr vk::PipelineColorBlendStateCreateInfo colorBlending {
		.logicOpEnable = vk::False,
		.logicOp =  vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};
	// 9. Fixed Function State
	// Dynamic States allow us to resize window without recreating pipeline
	static constexpr std::array<vk::DynamicState, 2> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};
	constexpr vk::PipelineDynamicStateCreateInfo dynamicStateInfo {
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data()
	};
	// 10. Pipeline Layout (Uniforms/Push Constants go here)
	static constexpr vk::PushConstantRange pushConstantRange {
		.stageFlags = vk::ShaderStageFlagBits::eVertex,
		.offset = 0,
		.size = pushConstantSize
	};
	constexpr vk::PipelineLayoutCreateInfo pipelineLayoutInfo {
		.setLayoutCount = 0,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstantRange
	};
	m_pipelineLayout = vk::raii::PipelineLayout(device.device(), pipelineLayoutInfo);
	// 11. Dynamic Rendering Info (Vulkan 1.3)
	const vk::Format colorFormat = swapchain.getImageFormat();

	const vk::PipelineRenderingCreateInfo pipelineRenderingInfo {
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &colorFormat,
		.depthAttachmentFormat = vk::Format::eUndefined // No depth buffer yet
	};
	// 12. Create Pipeline
	const vk::GraphicsPipelineCreateInfo pipelineInfo {
		.pNext = &pipelineRenderingInfo, // 11
		.stageCount = 2,
		.pStages = shaderStages, // 1, 2
		.pVertexInputState = &vertexInputInfo, // 3
		.pInputAssemblyState = &inputAssembly, // 4
		.pViewportState = &viewportState, // 5
		.pRasterizationState = &rasterizer, // 6
		.pMultisampleState = &multisampling, // 7
		.pColorBlendState = &colorBlending, // 8
		.pDynamicState = &dynamicStateInfo, // 9
		.layout = m_pipelineLayout, // 10
		.renderPass = nullptr, // Must be null for dynamic rendering
		.subpass = 0
	};

	m_pipeline = device.device().createGraphicsPipeline(nullptr, pipelineInfo);
	LOG_DEBUG("Graphics Pipeline created");
}

VulkanPipeline::~VulkanPipeline()
	{ LOG_DEBUG("Graphics Pipeline destroyed"); }
