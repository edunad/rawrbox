
#include <rawrbox/render/resources/gif.hpp>

namespace rawrbox {

	// Resource ----
	bool ResourceGIF::load(const std::vector<uint8_t>& buffer) {
		this->_texture = std::make_unique<rawrbox::TextureGIF>(this->filePath, buffer);
		return true;
	}

	void ResourceGIF::upload() {
		if (this->_texture == nullptr) return;
		this->_texture->upload();
	}

	rawrbox::TextureGIF* ResourceGIF::get() const {
		return this->_texture.get();
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
