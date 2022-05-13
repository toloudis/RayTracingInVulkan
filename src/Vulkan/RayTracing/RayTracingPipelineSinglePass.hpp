#pragma once

#include "Vulkan/Vulkan.hpp"
#include "RayTracingPipelineBase.hpp"
#include <memory>
#include <vector>

namespace Assets
{
	class Scene;
	class UniformBuffer;
}

namespace Vulkan
{
	class DescriptorSetManager;
	class ImageView;
	class PipelineLayout;
	class SwapChain;
}

namespace Vulkan::RayTracing
{
	class DeviceProcedures;
	class TopLevelAccelerationStructure;

	class RayTracingPipelineSinglePass final : public RayTracingPipelineBase
	{
	public:

		VULKAN_NON_COPIABLE(RayTracingPipelineSinglePass)

		RayTracingPipelineSinglePass(
			const DeviceProcedures& deviceProcedures,
			const SwapChain& swapChain,
			const TopLevelAccelerationStructure& accelerationStructure,
			const ImageView& accumulationImageView,
			const ImageView& outputImageView,
			const std::vector<Assets::UniformBuffer>& uniformBuffers,
			const Assets::Scene& scene);
		~RayTracingPipelineSinglePass();

	};

}
