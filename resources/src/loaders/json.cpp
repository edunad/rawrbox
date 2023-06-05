
#include <rawrbox/resources/loaders/json.hpp>

namespace rawrbox {
	// Resource ----
	ResourceJSON::~ResourceJSON() { this->json.reset(); }
	bool ResourceJSON::load(const std::vector<uint8_t>& buffer) {
		json = std::make_shared<nlohmann::json>(nlohmann::json::parse(buffer));
		return true;
	}

	// -------

	// Loader ----
	std::unique_ptr<rawrbox::Resource> JSONLoader::createEntry(uint32_t flags) {
		return std::make_unique<rawrbox::ResourceJSON>();
	}

	bool JSONLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".json";
	}
	// -------
} // namespace rawrbox
