#pragma once

#include <rawrbox/render/textures/base.hpp>

#include <filesystem>
#include <vector>

namespace rawrbox {
	// https://github.khronos.org/KTX-Specification/ktxspec.v2.html
	class TextureKTX2 : public rawrbox::TextureBase {
	protected:
		std::filesystem::path _filePath = "";
		virtual void internalLoad(const uint8_t* buffer, size_t bufferSize, bool useFallback);

	public:
		explicit TextureKTX2(const std::filesystem::path& filePath, bool useFallback = true);
		explicit TextureKTX2(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback = true);
		explicit TextureKTX2(const std::filesystem::path& filePath, const uint8_t* buffer, size_t bufferSize, bool useFallback = true);

		TextureKTX2(const TextureKTX2&) = delete;
		TextureKTX2(TextureKTX2&&) = delete;
		TextureKTX2& operator=(const TextureKTX2&) = delete;
		TextureKTX2& operator=(TextureKTX2&&) = delete;
		~TextureKTX2() override = default;
	};
} // namespace rawrbox