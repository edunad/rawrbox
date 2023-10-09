
#include <rawrbox/render_temp/resources/texture.hpp>
#include <rawrbox/render_temp/texture/atlas.hpp>
#include <rawrbox/render_temp/texture/gif.hpp>
#include <rawrbox/render_temp/texture/image.hpp>
#include <rawrbox/render_temp/texture/webp.hpp>

namespace rawrbox {
	// Resource ----
	ResourceTexture::~ResourceTexture() { this->_texture.reset(); }

	bool ResourceTexture::load(const std::vector<uint8_t>& buffer) {
		const bool isGIF = this->filePath.extension() == ".gif";
		if (isGIF) {
			this->_texture = std::make_unique<rawrbox::TextureGIF>(this->filePath, buffer);
			return true;
		}

		const bool isWEBP = this->filePath.extension() == ".webp";
		if (isWEBP) {
			this->_texture = std::make_unique<rawrbox::TextureWEBP>(this->filePath, buffer);
			return true;
		}

		if (flags) {
			this->_texture = std::make_unique<rawrbox::TextureAtlas>(this->filePath, buffer, flags); // Use flags for sprite size
		} else {
			this->_texture = std::make_unique<rawrbox::TextureImage>(this->filePath, buffer);
		}

		return true;
	}

	void ResourceTexture::upload() {
		if (this->_texture == nullptr) return;
		this->_texture->upload();
	}

	// -------

	// Loader ----
	std::unique_ptr<rawrbox::Resource> TextureLoader::createEntry() {
		return std::make_unique<rawrbox::ResourceTexture>();
	}

	bool TextureLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".png" ||
		       fileExtention == ".bmp" ||
		       fileExtention == ".jpg" ||
		       fileExtention == ".gif" ||
		       fileExtention == ".tga" ||
		       fileExtention == ".webp" ||
		       fileExtention == ".jpeg";
	}
	// -------
} // namespace rawrbox
