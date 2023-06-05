
#include <rawrbox/render/resources/texture.hpp>

namespace rawrbox {
	// Resource ----
	ResourceTexture::~ResourceTexture() { this->texture.reset(); }
	bool ResourceTexture::load(const std::vector<uint8_t>& buffer) {
		this->texture = std::make_shared<rawrbox::TextureImage>(this->filePath, buffer);
		return true;
	}

	void ResourceTexture::upload() {
		if (this->texture == nullptr) return;
		this->texture->upload();
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
