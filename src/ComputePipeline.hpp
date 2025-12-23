#pragma once
#include <vulkan/vulkan_raii.hpp>

class VulkanDevice;

class ComputePipeline
{
public:
	ComputePipeline(const VulkanDevice& device);
	~ComputePipeline();

	inline const vk::raii::Pipeline& getPipeline() const { return m_pipeline; }
	inline const vk::raii::PipelineLayout& getLayout() const { return m_pipelineLayout; }
	inline const vk::raii::DescriptorSetLayout& getDescriptorSetLayout() const { return m_descriptorSetLayout; }

private:
	vk::raii::DescriptorSetLayout m_descriptorSetLayout = nullptr;
	vk::raii::PipelineLayout m_pipelineLayout = nullptr;

	vk::raii::Pipeline m_pipeline = nullptr;
};
