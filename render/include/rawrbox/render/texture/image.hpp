#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>

#include <filesystem>
#include <string>
#include <vector>

namespace rawrbox {
	class TextureImage : public rawrbox::TextureBase {
	protected:
		std::vector<uint8_t> _pixels;

		bool _failedToLoad = false;
		bool _transparent = false;

		std::filesystem::path _filePath = "";
		std::string _name = "IMAGE-TEXTURE";

		void internalLoad(uint8_t* image, bool useFallback = true);

	public:
		explicit TextureImage(const std::filesystem::path& filePath, int forceChannels = 0, bool useFallback = true);
		explicit TextureImage(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, int forceChannels = 0, bool useFallback = true);
		explicit TextureImage(const uint8_t* buffer, int bufferSize, int forceChannels = 0, bool useFallback = true); // TO BE DECODED

		explicit TextureImage(const rawrbox::Vector2i& size, const uint8_t* buffer, int channels = 4);
		explicit TextureImage(const rawrbox::Vector2i& size, const std::vector<uint8_t>& buffer, int channels = 4);

		explicit TextureImage(const rawrbox::Vector2i& size, int channels = 4);

		TextureImage(const TextureImage&) = default;
		TextureImage(TextureImage&&) = delete;
		TextureImage& operator=(const TextureImage&) = delete;
		TextureImage& operator=(TextureImage&&) = delete;
		~TextureImage() override = default;

		// ------ PIXEL-UTILS
		virtual void updatePixels(const std::vector<uint8_t>& buffer);
		virtual void updatePixels(const uint8_t* buffer, size_t size);

		virtual void setName(const std::string& name);
		[[nodiscard]] bool hasTransparency() const override;
		// --------------------

		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Count) override;
	};
} // namespace rawrbox
