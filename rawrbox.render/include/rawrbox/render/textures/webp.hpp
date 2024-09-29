#pragma once

#include <rawrbox/render/textures/animated.hpp>

namespace rawrbox {
	class TextureWEBP : public rawrbox::TextureAnimatedBase {
	protected:
		void internalLoad(const std::vector<uint8_t>& data, bool useFallback = true) override;
		void internalLoad(const uint8_t* buffer, size_t bufferSize, bool useFallback = true);

	public:
		explicit TextureWEBP(const std::filesystem::path& filePath, bool useFallback = true);
		explicit TextureWEBP(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback = true);
		explicit TextureWEBP(const std::filesystem::path& filePath, const uint8_t* buffer, size_t bufferSize, bool useFallback = true);
	};
} // namespace rawrbox
