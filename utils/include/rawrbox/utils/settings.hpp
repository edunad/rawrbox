#pragma once

#include <nlohmann/json.hpp>

namespace rawrbox {
	class Settings {
	protected:
		nlohmann::json _settings = {};

	public:
		Settings() = default;
		Settings(const Settings &) = default;
		Settings(Settings &&) = delete;
		Settings &operator=(const Settings &) = default;
		Settings &operator=(Settings &&) = delete;
		virtual ~Settings() = default;

		virtual void load();
		virtual void save();

		[[nodiscard]] virtual const std::string getVersion() const;
		[[nodiscard]] virtual const std::string getFileName() const;

		virtual nlohmann::json getDefaults();
		virtual nlohmann::json &getSettings();
	};
} // namespace rawrbox
