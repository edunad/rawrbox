#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#include <stb/image.hpp>

#include <filesystem>
#include <string>

namespace rawrbox {
	class TextureImage : public rawrbox::TextureBase {
	private:
		std::vector<unsigned char> _pixels;

		bool _failedToLoad = false;
		std::filesystem::path _filePath = "";
		std::string _name = "IMAGE-TEXTURE";

		void internalLoad(stbi_uc* image, bool useFallback = true);

	public:
		explicit TextureImage(const std::filesystem::path& filePath, bool useFallback = true);
		explicit TextureImage(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback = true);

		// ------ PIXEL-UTILS
		virtual void setName(const std::string& name);
		virtual void resize(const rawrbox::Vector2i& newsize);
		// --------------------

		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Count) override;
	};
} // namespace rawrbox
