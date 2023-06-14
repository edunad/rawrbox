#pragma once

#include <rawrbox/bass/sound/base.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceBASS : public rawrbox::Resource {
		rawrbox::SoundBase* _sound = nullptr;

	public:
		bool load(const std::vector<uint8_t>& buffer) override;
		[[nodiscard]] rawrbox::SoundBase* get() const;

		ResourceBASS() = default;
		~ResourceBASS() override;
		ResourceBASS(const ResourceBASS&) = default;
		ResourceBASS(ResourceBASS&&) = delete;
		ResourceBASS& operator=(const ResourceBASS&) = default;
		ResourceBASS& operator=(ResourceBASS&&) = delete;
	};

	class BASSLoader : public rawrbox::Loader {
	public:
		BASSLoader();
		~BASSLoader() override;

		BASSLoader(const BASSLoader&) = delete;
		BASSLoader(BASSLoader&&) = delete;
		BASSLoader& operator=(const BASSLoader&) = delete;
		BASSLoader& operator=(BASSLoader&&) = delete;

		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override;
		bool canLoad(const std::string& fileExtention) override;
	};
} // namespace rawrbox
