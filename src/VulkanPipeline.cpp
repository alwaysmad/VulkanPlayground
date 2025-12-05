#include "VulkanPipeline.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapchain.hpp"
#include "DebugOutput.hpp"
#include <fstream>

[[nodiscard]] inline static vk::raii::ShaderModule loadShaderModule(
		const vk::raii::Device& device,
		const std::string& filename )
{
	// 1. Open file
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open()) { throw std::runtime_error("Failed to open shader file: " + filename); }
	// 2. Read file size
	const size_t fileSize = file.tellg();
	// 3. Read data
	std::vector<char> buffer(fileSize);
	file.seekg(0, std::ios::beg);
	file.read(buffer.data(), fileSize);
	file.close();
	// 4. Create Shader Module
	const vk::ShaderModuleCreateInfo createInfo {
		.codeSize = fileSize * sizeof(char),
		.pCode = reinterpret_cast<const uint32_t*>(buffer.data())
	};
	return vk::raii::ShaderModule{ device, createInfo };
}

VulkanPipeline::VulkanPipeline(
		const VulkanDevice& device,
		const VulkanSwapchain& swapchain,
		const std::string& shaderPath) :
	m_device(device),
	m_pipelineLayout(nullptr),
	m_pipeline(nullptr)
{
	// 1. Load Shaders
	vk::raii::ShaderModule shaderModule = loadShaderModule(m_device.device(), shaderPath);
	// 2. Shader Stages
	vk::PipelineShaderStageCreateInfo shaderStages[] = {
		{
			.stage = vk::ShaderStageFlagBits::eVertex,
			.module = shaderModule,
			.pName = "vertMain" // Entry point from Slang shader
		},
		{
			.stage = vk::ShaderStageFlagBits::eFragment,
			.module = shaderModule,
			.pName = "fragMain" // Entry point from your Slang shader
		}
	};
	// 3. Vertex input
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo { }; // Empty for now (hardcoded in shader)
	// 4. Input assembly
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly {
		.topology = vk::PrimitiveTopology::eTriangleList,
		.primitiveRestartEnable = vk::False
	};
	// 5. Viewport
	vk::PipelineViewportStateCreateInfo viewportState {
		.viewportCount = 1,
		.scissorCount = 1
	};
	// 6. Rasterizer
	vk::PipelineRasterizationStateCreateInfo rasterizer {
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
	vk::PipelineMultisampleStateCreateInfo multisampling {
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = vk::False
	};
	// 8. Color blending
	vk::PipelineColorBlendAttachmentState colorBlendAttachment {
		.blendEnable = vk::False,
		.colorWriteMask
			= vk::ColorComponentFlagBits::eR
			| vk::ColorComponentFlagBits::eG
			| vk::ColorComponentFlagBits::eB
			| vk::ColorComponentFlagBits::eA
	};

	vk::PipelineColorBlendStateCreateInfo colorBlending {
		.logicOpEnable = vk::False,
		.logicOp =  vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments =  &colorBlendAttachment
	};
	// 9. Fixed Function State
	// Dynamic States allow us to resize window without recreating pipeline
	std::vector<vk::DynamicState> dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};

	vk::PipelineDynamicStateCreateInfo dynamicStateInfo {
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data()
	};
	// 10. Pipeline Layout (Uniforms/Push Constants go here)
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo {
		.setLayoutCount = 0,
		.pushConstantRangeCount = 0
	};
	m_pipelineLayout = vk::raii::PipelineLayout(device.device(), pipelineLayoutInfo);
	// 11. Dynamic Rendering Info (Vulkan 1.3)
	vk::Format colorFormat = swapchain.getImageFormat();

	vk::PipelineRenderingCreateInfo pipelineRenderingInfo {
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &colorFormat,
		.depthAttachmentFormat = vk::Format::eUndefined // No depth buffer yet
	};
	// 12. Create Pipeline
	vk::GraphicsPipelineCreateInfo pipelineInfo {
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
{
	LOG_DEBUG("Graphics Pipeline destroyed");
}
