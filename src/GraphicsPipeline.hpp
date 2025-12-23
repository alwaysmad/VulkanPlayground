#pragma once

class VulkanDevice;

class GraphicsPipeline
{
public:
	GraphicsPipeline(const VulkanDevice&, vk::Format colorFormat, vk::Format depthFormat);
	~GraphicsPipeline();

	inline const vk::raii::Pipeline& getPipeline() const { return m_pipeline; }
	inline const vk::raii::PipelineLayout& getLayout() const { return m_pipelineLayout; }

private:
	vk::raii::PipelineLayout m_pipelineLayout = nullptr;
	vk::raii::Pipeline m_pipeline = nullptr;
};
