#pragma once

#include <rawrbox/bass/manager.hpp>
#include <rawrbox/bass/sound/base.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceBASS : public rawrbox::Resource {
	public:
		std::shared_ptr<rawrbox::SoundBase> sound = nullptr;
		~ResourceBASS() override {
			this->sound = nullptr;
		}

		bool load(const std::vector<uint8_t>& buffer) override {
			auto p = this->filePath.generic_string();
			if (p.starts_with("https://") || p.starts_with("http://")) throw std::runtime_error("[RawrBox-BASS] Use 'rawrbox::BASS::loadSoundHTPP' instead!\n");

			try {
				this->sound = rawrbox::BASS::loadSound(this->filePath, this->flags);
			} catch (const std::exception& e) {
				fmt::print("{}\n", e.what());
				fmt::print("  └── Loading fallback sound!\n");

				this->sound = rawrbox::BASS::loadSound("./content/sound/error.ogg", this->flags);
				if (this->sound == nullptr) return false;
			}

			return true;
		}
	};

	class BASSLoader : public rawrbox::Loader {
	public:
		BASSLoader() { rawrbox::BASS::initialize(); }
		~BASSLoader() override { rawrbox::BASS::shutdown(); };

		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override {
			return std::make_unique<rawrbox::ResourceBASS>();
		}

		bool canLoad(const std::string& fileExtention) override {
			return fileExtention == ".mp3" ||
			       fileExtention == ".mp4" ||
			       fileExtention == ".ogg" ||
			       fileExtention == ".wav";
		}
	};
} // namespace rawrbox
