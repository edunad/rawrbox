
#include <rawrbox/render/resources/webm/webm.hpp>
#include <rawrbox/render/texture/webm/webm.hpp>

namespace rawrbox {
	// Resource ----
	ResourceWEBM::~ResourceWEBM() { this->_texture.reset(); }
	bool ResourceWEBM::load(const std::vector<uint8_t>& /*buffer*/) {
		this->_texture = std::make_unique<rawrbox::TextureWEBM>(this->filePath);
		return true;
	}

	void ResourceWEBM::upload() {
		if (this->_texture == nullptr) return;
		this->_texture->upload();
	}

	[[nodiscard]] rawrbox::TextureBase* ResourceWEBM::get() const {
		return this->_texture.get();
	}
	// -------

	// Loader ----
	WEBMLoader::WEBMLoader() {
		rawrbox::WEBMDecoder::init();
	}

	WEBMLoader::~WEBMLoader() {
		rawrbox::WEBMDecoder::shutdown();
	}

	std::unique_ptr<rawrbox::Resource> WEBMLoader::createEntry() {
		return std::make_unique<rawrbox::ResourceWEBM>();
	}

	bool WEBMLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".webm";
	}
	// -------
} // namespace rawrbox
