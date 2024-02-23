#include <rawrbox/utils/json.hpp>
#include <rawrbox/utils/settings.hpp>

#include <magic_enum.hpp>

#include <fmt/format.h>

#include <fstream>

namespace rawrbox {
	glz::json_t Settings::getDefaults() {
		throw this->_logger->error("Implement getDefaults");
	}

	std::string Settings::getVersion() const {
		throw this->_logger->error("Implement getVersion");
	}

	std::string Settings::getFileName() const {
		throw this->_logger->error("Implement getFileName");
	}

	void Settings::save() {
		auto fileName = this->getFileName();

		std::ofstream out(fileName);
		if (!out.is_open()) throw this->_logger->error("Failed to save settings '{}'", fileName);

		out << glz::write<glz::opts{.prettify = true}>(this->_settings);
		out.close();
	}

	void Settings::load(std::string data) {
		_settings.reset();

		if (data.empty()) {
			std::ifstream settingsFile(this->getFileName());
			data = std::string((std::istreambuf_iterator<char>(settingsFile)), std::istreambuf_iterator<char>());
		}

		if (!data.empty()) {
			auto err = glz::read_json(this->_settings, data);
			if (err != glz::error_code::none) {
				throw _logger->error("Failed to load '{}' ──> {}", this->getFileName(), magic_enum::enum_name(err.ec));
			}

			// Validate version
			if (_settings.contains("VERSION") && _settings["VERSION"].get<std::string>() != this->getVersion()) {
				this->_logger->warn("Migrating settings to version '{}'", this->getVersion());
				auto diff = rawrbox::JSONUtils::diff(this->_settings, getDefaults());

				std::vector<rawrbox::JSONDiff> fixedDiff = {};
				for (auto& itm : diff) {
					if (itm.op == rawrbox::JSONDiffOp::REPLACE && itm.path != "/VERSION") continue;
					fixedDiff.push_back(itm);
				}

				rawrbox::JSONUtils::patch(this->_settings, fixedDiff);
				this->_settings["VERSION"] = this->getVersion(); // For settings version comparison
				this->save();
			} else {
				this->_logger->warn("Missing version, cannot migrate! Adding current version, some things might be broken!");
				this->_settings["VERSION"] = this->getVersion(); // For settings version comparison
			}

		} else {
			this->_settings = this->getDefaults();
			this->_settings["VERSION"] = this->getVersion(); // For settings version comparison

			this->save();
		}
	}

	glz::json_t& Settings::getSettings() {
		return this->_settings;
	}
} // namespace rawrbox
