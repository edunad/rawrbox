#pragma once

#include <rawrbox/bass/sound/base.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceBASS : public rawrbox::Resource {
	public:
		std::shared_ptr<rawrbox::SoundBase> sound = nullptr;

		~ResourceBASS() override;
		bool load(const std::vector<uint8_t>& buffer) override;
	};

	class BASSLoader : public rawrbox::Loader {
	public:
		BASSLoader();
		~BASSLoader() override;

		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override;
		bool canLoad(const std::string& fileExtention) override;
	};
} // namespace rawrbox
