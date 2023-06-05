
#include <rawrbox/render/resources/gif.hpp>

namespace rawrbox {

	// Resource ----
	ResourceGIF::~ResourceGIF() { this->texture.reset(); }
	bool ResourceGIF::load(const std::vector<uint8_t>& buffer) {
		this->texture = std::make_shared<rawrbox::TextureGIF>(this->filePath, buffer);
		return true;
	}

	void ResourceGIF::upload() {
		if (this->texture == nullptr) return;
		this->texture->upload();
	}

	// -------

	// Loader ----
	std::unique_ptr<rawrbox::Resource> GIFLoader::createEntry(uint32_t flags) {
		return std::make_unique<rawrbox::ResourceGIF>();
	}

	bool GIFLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".gif";
	}
	// -------
} // namespace rawrbox
