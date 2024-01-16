#pragma once

#include <rawrbox/utils/logger.hpp>

#include <nlohmann/json.hpp>

namespace rawrbox {
	class Settings {
	protected:
		nlohmann::json _settings = {};

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

		virtual void load();
		virtual void save();

		[[nodiscard]] virtual const std::string getVersion() const;
		[[nodiscard]] virtual const std::string getFileName() const;

		virtual nlohmann::json getDefaults();
		virtual nlohmann::json& getSettings();
	};
} // namespace rawrbox
