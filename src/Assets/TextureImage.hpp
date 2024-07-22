#pragma once

#include <memory>

namespace Vulkan
{
	class CommandPool;
	class DeviceMemory;
	class Image;
	class ImageView;
	class Sampler;
	class VolumeImage;
	class VolumeImageView;
}

namespace Assets
{
	class Texture;

	class TextureImage final
	{
	public:
		TextureImage(const TextureImage &) = delete;
		TextureImage(TextureImage &&) = delete;
		TextureImage &operator=(const TextureImage &) = delete;
		TextureImage &operator=(TextureImage &&) = delete;

		TextureImage(Vulkan::CommandPool &commandPool, const Texture &texture);
		~TextureImage();

		const Vulkan::ImageView &ImageView() const { return *imageView_; }
		const Vulkan::Sampler &Sampler() const { return *sampler_; }

	private:
		std::unique_ptr<Vulkan::Image> image_;
		std::unique_ptr<Vulkan::DeviceMemory> imageMemory_;
		std::unique_ptr<Vulkan::ImageView> imageView_;
		std::unique_ptr<Vulkan::Sampler> sampler_;
	};

	class VolumeTexture;

	class VolumeTextureImage final
	{
	public:
		VolumeTextureImage(const VolumeTextureImage &) = delete;
		VolumeTextureImage(VolumeTextureImage &&) = delete;
		VolumeTextureImage &operator=(const VolumeTextureImage &) = delete;
		VolumeTextureImage &operator=(VolumeTextureImage &&) = delete;

		VolumeTextureImage(Vulkan::CommandPool &commandPool, const VolumeTexture &texture);
		~VolumeTextureImage();

		const Vulkan::VolumeImageView &ImageView() const { return *imageView_; }
		const Vulkan::Sampler &Sampler() const { return *sampler_; }

	private:
		std::unique_ptr<Vulkan::VolumeImage> image_;
		std::unique_ptr<Vulkan::DeviceMemory> imageMemory_;
		std::unique_ptr<Vulkan::VolumeImageView> imageView_;
		std::unique_ptr<Vulkan::Sampler> sampler_;
	};

}
