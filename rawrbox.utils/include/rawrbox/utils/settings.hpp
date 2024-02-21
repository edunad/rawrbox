#pragma once

#include <rawrbox/utils/logger.hpp>

#include <glaze/glaze.hpp>

namespace rawrbox {
	class Settings {
	protected:
		glz::json_t _settings = {};

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Settings");
		// -------------

	public:
		Settings() = default;
		Settings(const Settings&) = delete;
		Settings(Settings&&) = delete;
		Settings& operator=(const Settings&) = delete;
		Settings& operator=(Settings&&) = delete;
		virtual ~Settings() = default;

		virtual void load(std::string data = "");
		virtual void save();

		[[nodiscard]] virtual std::string getVersion() const;
		[[nodiscard]] virtual std::string getFileName() const;

		virtual glz::json_t getDefaults();
		virtual glz::json_t& getSettings();
	};
} // namespace rawrbox
