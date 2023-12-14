
#include <rawrbox/webm/resources/webm.hpp>
#include <rawrbox/webm/textures/webm.hpp>

namespace rawrbox {
	// Resource ----
	ResourceWEBM::~ResourceWEBM() { this->_texture.reset(); }
	bool ResourceWEBM::load(const std::vector<uint8_t>& /*buffer*/) {
		this->_texture = std::make_unique<rawrbox::TextureWEBM>(this->filePath, this->flags);
		return true;
	}

	void ResourceWEBM::upload() {
		if (this->_texture == nullptr) return;
		this->_texture->upload();
	}
	// -------

	// Loader ----
	WEBMLoader::WEBMLoader() {
		rawrbox::WEBMDecoder::init(rawrbox::VIDEO_CODEC::VIDEO_VP9);
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
