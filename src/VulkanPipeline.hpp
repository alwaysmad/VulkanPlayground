#pragma once
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <string>

class VulkanDevice;
class VulkanSwapchain;

class VulkanPipeline
{
public:
	VulkanPipeline(const VulkanDevice&, 
			const VulkanSwapchain&, 
			const std::string&);

	~VulkanPipeline();

	inline const vk::raii::Pipeline& getPipeline() const { return m_pipeline; }
	inline const vk::raii::PipelineLayout& getLayout() const { return m_pipelineLayout; }

private:
	const VulkanDevice& m_device;

	vk::raii::PipelineLayout m_pipelineLayout;
	vk::raii::Pipeline m_pipeline;
};
