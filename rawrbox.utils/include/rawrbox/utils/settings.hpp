#pragma once

#include <rawrbox/utils/logger.hpp>
#include <rawrbox/utils/settings.hpp>

#include <glaze/glaze.hpp>
#include <magic_enum.hpp>

namespace rawrbox {
	template <typename T>
	concept hasJSONVersion = requires(T t) { t.VERSION; };

	template <typename T>
		requires(hasJSONVersion<T> && glz::write_json_supported<T> && glz::read_json_supported<T>)
	class Settings {
	protected:
		T _settings = {};

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Settings");
		// -------------

		virtual void parse() {
			T defaults = {};

			this->_settings.VERSION = defaults.VERSION; // For settings version comparison
			this->save();
		}

	public:
		Settings() = default;
		Settings(const Settings&) = delete;
		Settings(Settings&&) = delete;
		Settings& operator=(const Settings&) = delete;
		Settings& operator=(Settings&&) = delete;
		virtual ~Settings() = default;

		virtual void load(const std::string& rawData) {
			if (!rawData.empty()) {
				auto err = glz::read<glz::opts{.comments = 1U, .error_on_unknown_keys = 0U, .skip_null_members = 1U, .error_on_missing_keys = 0U, .allow_conversions = 1U}>(this->_settings, rawData);
				if (err != glz::error_code::none) throw this->_logger->error("Failed to load settings ──> {}", magic_enum::enum_name(err.ec));
			} else {
				this->_settings = {};
			}

			this->parse();
		}

		virtual void load() {
			auto filePath = this->getFileName();

			if (std::filesystem::exists(this->getFileName())) {
				auto err = glz::read_file_json<glz::opts{.comments = 1U, .error_on_unknown_keys = 0U, .skip_null_members = 1U, .error_on_missing_keys = 0U, .allow_conversions = 1U}>(this->_settings, filePath.generic_string(), std::string{});
				if (err != glz::error_code::none) throw _logger->error("Failed to load '{}' ──> {}", filePath.generic_string(), magic_enum::enum_name(err.ec));
			} else {
				this->_settings = {};
			}

			this->parse();
		}

		virtual void save() {
			auto fileName = this->getFileName().generic_string();
			auto ec = glz::write_file_json<glz::opts{.comments = 1U, .prettify = 1U, .allow_conversions = 1U}>(this->_settings, fileName, std::string{});
			if (ec != glz::error_code::none) throw this->_logger->error("Failed to save settings '{}'", fileName);
		}

		[[nodiscard]] virtual std::filesystem::path getFileName() const {
			throw this->_logger->error("Implement getFileName");
		}

		[[nodiscard]] virtual T& getSettings() {
			return this->_settings;
		}
	};
} // namespace rawrbox
