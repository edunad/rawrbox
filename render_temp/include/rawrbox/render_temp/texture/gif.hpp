#pragma once

#include <rawrbox/render_temp/texture/animated.hpp>

namespace rawrbox {
	class TextureGIF : public rawrbox::TextureAnimatedBase {
	private:
		void internalLoad(const std::vector<uint8_t>& data, bool useFallback = true) override;

	public:
		explicit TextureGIF(const std::filesystem::path& filePath, bool useFallback = true);
		explicit TextureGIF(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback = true);
	};
} // namespace rawrbox
