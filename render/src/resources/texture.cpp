
#include <rawrbox/render/resources/texture.hpp>

namespace rawrbox {
	// Resource ----
	ResourceTexture::~ResourceTexture() { this->_texture.reset(); }
	bool ResourceTexture::load(const std::vector<uint8_t>& buffer) {
		this->_texture = std::make_unique<rawrbox::TextureImage>(this->filePath, buffer);
		return true;
	}

	void ResourceTexture::upload() {
		if (this->_texture == nullptr) return;
		this->_texture->upload();
	}

	rawrbox::TextureImage* ResourceTexture::get() const {
		return this->_texture.get();
	}
	// -------

	// Loader ----
	std::unique_ptr<rawrbox::Resource> TextureLoader::createEntry(uint32_t flags) {
		return std::make_unique<rawrbox::ResourceTexture>();
	}

	bool TextureLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".png" ||
		       fileExtention == ".bmp" ||
		       fileExtention == ".jpg" ||
		       fileExtention == ".jpeg";
	}
	// -------
} // namespace rawrbox
