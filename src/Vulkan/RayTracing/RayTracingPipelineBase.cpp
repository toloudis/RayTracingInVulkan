#include "RayTracingPipelineBase.hpp"
#include "DeviceProcedures.hpp"
#include "TopLevelAccelerationStructure.hpp"
#include "Assets/Scene.hpp"
#include "Assets/UniformBuffer.hpp"
#include "Utilities/Exception.hpp"
#include "Vulkan/Buffer.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/DescriptorBinding.hpp"
#include "Vulkan/DescriptorSetManager.hpp"
#include "Vulkan/DescriptorSets.hpp"
#include "Vulkan/ImageView.hpp"
#include "Vulkan/PipelineLayout.hpp"
#include "Vulkan/ShaderModule.hpp"
#include "Vulkan/SwapChain.hpp"

namespace Vulkan::RayTracing {

RayTracingPipelineBase::RayTracingPipelineBase(
	const DeviceProcedures& deviceProcedures,
	const SwapChain& swapChain,
	const TopLevelAccelerationStructure& accelerationStructure,
	const ImageView& accumulationImageView,
	const ImageView& outputImageView,
	const std::vector<Assets::UniformBuffer>& uniformBuffers,
	const Assets::Scene& scene) :
	swapChain_(swapChain)
{
	init(deviceProcedures, swapChain, accelerationStructure, accumulationImageView, outputImageView, uniformBuffers, scene);
}

RayTracingPipelineBase::~RayTracingPipelineBase()
{
	uninit();
}

VkDescriptorSet RayTracingPipelineBase::DescriptorSet(const uint32_t index) const
{
	return descriptorSetManager_->DescriptorSets().Handle(index);
}

}
