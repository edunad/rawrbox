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
		explicit TextureImage(uint8_t* buffer, int bufferSize, int forceChannels = 0, bool useFallback = true);
		explicit TextureImage(const rawrbox::Vector2i& size, uint8_t* buffer, int channels = 4);

		TextureImage(const TextureImage&) = default;
		TextureImage(TextureImage&&) = delete;
		TextureImage& operator=(const TextureImage&) = delete;
		TextureImage& operator=(TextureImage&&) = delete;
		~TextureImage() override = default;

		// ------ PIXEL-UTILS
		virtual void setName(const std::string& name);
		[[nodiscard]] bool hasTransparency() const override;
		// --------------------

		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Count) override;
	};
} // namespace rawrbox
