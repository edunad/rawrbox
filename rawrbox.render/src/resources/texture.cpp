
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/textures/atlas.hpp>
#include <rawrbox/render/textures/gif.hpp>
#include <rawrbox/render/textures/image.hpp>
#include <rawrbox/render/textures/webp.hpp>

namespace rawrbox {
	// Resource ----
	ResourceTexture::~ResourceTexture() { this->_texture.reset(); }

	bool ResourceTexture::load(const std::vector<uint8_t>& buffer) {
		const rawrbox::TEXTURE_TYPE type = this->filePath.generic_string().rfind(".vertex.") != std::string::npos ? rawrbox::TEXTURE_TYPE::VERTEX : rawrbox::TEXTURE_TYPE::PIXEL;
		auto extension = this->filePath.extension();

		if (extension == ".gif") {
			this->_texture = std::make_unique<rawrbox::TextureGIF>(this->filePath, buffer);
		} else if (extension == ".webp") {
			this->_texture = std::make_unique<rawrbox::TextureWEBP>(this->filePath, buffer);
		} else if (flags != 0U) {
			this->_texture = std::make_unique<rawrbox::TextureAtlas>(this->filePath, buffer, flags); // Use flags for sprite size
		} else {
			this->_texture = std::make_unique<rawrbox::TextureImage>(this->filePath, buffer);
		}

		this->_texture->setType(type);
		this->_texture->setName(this->filePath.filename().generic_string());
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

	bool TextureLoader::supportsBuffer(const std::string& /*fileExtention*/) { return true; }
	// -------
} // namespace rawrbox
