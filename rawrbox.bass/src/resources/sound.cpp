
#include <rawrbox/bass/manager.hpp>
#include <rawrbox/bass/resources/sound.hpp>

namespace rawrbox {
	// Resource ----
	bool ResourceBASS::load(const std::vector<uint8_t>& /*buffer*/) {
		auto p = this->filePath.generic_string();
		bool http = p.starts_with("https://") || p.starts_with("http://");

		rawrbox::SoundBase* loaded = nullptr;
		try {
			if (http) {
				loaded = rawrbox::BASS::loadHTTPSound(this->filePath.generic_string(), this->flags);
			} else {
				loaded = rawrbox::BASS::loadSound(this->filePath, this->flags);
			}
		} catch (const std::exception& e) {
			fmt::print("\n\t{}\n\t\t └── Loading fallback sound!\n", e.what());
			loaded = rawrbox::BASS::loadSound("./assets/sound/error.ogg", this->flags);
		}

		return loaded != nullptr;
	}
	// -------

	// Loader ----
	BASSLoader::BASSLoader() { rawrbox::BASS::initialize(); }
	BASSLoader::~BASSLoader() { rawrbox::BASS::shutdown(); };

	std::unique_ptr<rawrbox::Resource> BASSLoader::createEntry() {
		return std::make_unique<rawrbox::ResourceBASS>();
	}

	bool BASSLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".mp3" ||
		       fileExtention == ".mp4" ||
		       fileExtention == ".ogg" ||
		       fileExtention == ".wav";
	}

	bool BASSLoader::supportsBuffer(const std::string& /*fileExtention*/) { return false; };
	// ----
} // namespace rawrbox
