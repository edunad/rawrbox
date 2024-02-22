
#include <rawrbox/resources/loaders/json.hpp>

#include <magic_enum.hpp>

namespace rawrbox {
	// Resource ----
	bool ResourceJSON::load(const std::vector<uint8_t>& buffer) {
		auto err = glz::read_json(this->_json, buffer);
		if (err != glz::error_code::none) {
			throw rawrbox::Logger::err("ResourceJSON", "Failed to load '{}' ──> {}\n", this->filePath.generic_string(), magic_enum::enum_name(err.ec));
		}

		return true;
	}

	const glz::json_t& ResourceJSON::get() const {
		return this->_json;
	}
	// -------

	// Loader ----
	std::unique_ptr<rawrbox::Resource> JSONLoader::createEntry() {
		return std::make_unique<rawrbox::ResourceJSON>();
	}

	bool JSONLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".json";
	}

	bool JSONLoader::supportsBuffer(const std::string& /*fileExtention*/) { return true; };
	// -------
} // namespace rawrbox
