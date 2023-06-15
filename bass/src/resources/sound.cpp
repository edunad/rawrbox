
#include <rawrbox/bass/manager.hpp>
#include <rawrbox/bass/resources/sound.hpp>

namespace rawrbox {
	// Resource ----
	bool ResourceBASS::load(const std::vector<uint8_t>& buffer) {
		auto p = this->filePath.generic_string();
		if (p.starts_with("https://") || p.starts_with("http://")) throw std::runtime_error("[RawrBox-BASS] Use 'rawrbox::BASS::loadSoundHTPP' instead!\n");

		try {
			this->_sound = rawrbox::BASS::loadSound(this->filePath, this->flags);
		} catch (const std::exception& e) {
			fmt::print("{}\n", e.what());
			fmt::print("  └── Loading fallback sound!\n");

			this->_sound = rawrbox::BASS::loadSound("./content/sound/error.ogg", this->flags);
			if (this->_sound == nullptr) return false;
		}

		return true;
	}
	rawrbox::SoundBase* ResourceBASS::get() const { return this->_sound; }
	ResourceBASS::~ResourceBASS() { this->_sound = nullptr; }
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
	// ----
} // namespace rawrbox
