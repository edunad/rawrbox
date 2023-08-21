#pragma once

#include <rawrbox/bass/sound/base.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceBASS : public rawrbox::Resource {
	public:
		bool load(const std::vector<uint8_t>& buffer) override;
	};

	class BASSLoader : public rawrbox::Loader {
	public:
		BASSLoader();
		~BASSLoader() override;

		BASSLoader(const BASSLoader&) = delete;
		BASSLoader(BASSLoader&&) = delete;
		BASSLoader& operator=(const BASSLoader&) = delete;
		BASSLoader& operator=(BASSLoader&&) = delete;

		template <class T>
		T* getFile(const std::filesystem::path& /*filePath*/) {
			throw std::runtime_error("[RawrBox-ResourceBASS] Use BASS:loadSound / BASS:loadHTTPSound instead");
		}

		std::unique_ptr<rawrbox::Resource> createEntry() override;
		bool canLoad(const std::string& fileExtention) override;
	};
} // namespace rawrbox
