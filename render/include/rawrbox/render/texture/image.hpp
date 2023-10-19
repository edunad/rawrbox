#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace rawrbox {
	class TextureImage : public rawrbox::TextureBase {
	protected:
		std::filesystem::path _filePath = "";
		void internalLoad(uint8_t* image, bool useFallback = true);

	public:
		explicit TextureImage(const std::filesystem::path& filePath, bool useFallback = true);
		explicit TextureImage(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback = true);
		explicit TextureImage(const uint8_t* buffer, int bufferSize, bool useFallback = true); // TO BE DECODED

		explicit TextureImage(const rawrbox::Vector2i& size, const uint8_t* buffer, int channels = 4);
		explicit TextureImage(const rawrbox::Vector2i& size, const std::vector<uint8_t>& buffer, int channels = 4);

		explicit TextureImage(const rawrbox::Vector2i& size, int channels = 4);

		TextureImage(const TextureImage&) = default;
		TextureImage(TextureImage&&) = delete;
		TextureImage& operator=(const TextureImage&) = delete;
		TextureImage& operator=(TextureImage&&) = delete;
		~TextureImage() override = default;
	};
} // namespace rawrbox
