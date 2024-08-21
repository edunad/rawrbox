#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/textures/base.hpp>

#include <filesystem>
#include <vector>

namespace rawrbox {
	class TextureImage : public rawrbox::TextureBase {
	protected:
		std::filesystem::path _filePath = "";

	public:
		explicit TextureImage(const std::filesystem::path& filePath, bool useFallback = true);
		explicit TextureImage(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback = true);

		explicit TextureImage(const uint8_t* buffer, int bufferSize, bool useFallback = true);

		explicit TextureImage(const rawrbox::Vector2u& size, const uint8_t* buffer, uint8_t channels = (uint8_t)4);
		explicit TextureImage(const rawrbox::Vector2u& size, const std::vector<uint8_t>& buffer, uint8_t channels = (uint8_t)4);

		explicit TextureImage(const rawrbox::Vector2u& size, uint8_t channels = (uint8_t)4);

		TextureImage(const TextureImage&) = delete;
		TextureImage(TextureImage&&) = delete;
		TextureImage& operator=(const TextureImage&) = delete;
		TextureImage& operator=(TextureImage&&) = delete;
		~TextureImage() override = default;
	};
} // namespace rawrbox
