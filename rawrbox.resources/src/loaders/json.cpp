
#include <rawrbox/resources/loaders/json.hpp>

namespace rawrbox {
	// Resource ----
	ResourceJSON::~ResourceJSON() { this->_json.reset(); }

	bool ResourceJSON::load(const std::vector<uint8_t>& buffer) {
		this->_json = std::make_unique<nlohmann::json>(nlohmann::json::parse(buffer, nullptr, true, true));
		return true;
	}

	nlohmann::json* ResourceJSON::get() const {
		return this->_json.get();
	}
	// -------

	// Loader ----
	std::unique_ptr<rawrbox::Resource> JSONLoader::createEntry() {
		return std::make_unique<rawrbox::ResourceJSON>();
	}

	bool JSONLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".json";
	}

	bool JSONLoader::supportsBuffer() { return true; };
	// -------
} // namespace rawrbox
