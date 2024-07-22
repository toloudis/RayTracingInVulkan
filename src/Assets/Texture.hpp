#pragma once

#include "Vulkan/Sampler.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Assets {
	class Texture final {
	public:
		static Texture LoadTexture(const std::string& filename, const Vulkan::SamplerConfig& samplerConfig);

		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) = delete;

		Texture() = default;
		Texture(const Texture&) = default;
		Texture(Texture&&) = default;
		~Texture() = default;

		const unsigned char* Pixels() const {
			return pixels_.get();
		}
		int Width() const {
			return width_;
		}
		int Height() const {
			return height_;
		}

	private:
		Texture(int width, int height, int channels, unsigned char* pixels);

		Vulkan::SamplerConfig samplerConfig_;
		int width_;
		int height_;
		int channels_;
		std::unique_ptr<unsigned char, void (*)(void*)> pixels_;
	};

	class VolumeTexture final {
	public:
		VolumeTexture(int width, int height, int depth, const std::vector<uint16_t>& pixels);

		VolumeTexture& operator=(const VolumeTexture&) = delete;
		VolumeTexture& operator=(VolumeTexture&&) = delete;

		VolumeTexture() = default;
		VolumeTexture(const VolumeTexture&) = default;
		VolumeTexture(VolumeTexture&&) = default;
		~VolumeTexture() = default;

		const uint16_t* Pixels() const {
			return pixels_.data();
		}
		int Width() const {
			return width_;
		}
		int Height() const {
			return height_;
		}
		int Depth() const {
			return depth_;
		}

	private:
		Vulkan::SamplerConfig samplerConfig_;
		int width_;
		int height_;
		int depth_;
		int channels_;
		std::vector<uint16_t> pixels_;
	};
}
