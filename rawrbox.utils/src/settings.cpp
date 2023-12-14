#include <rawrbox/utils/settings.hpp>

#include <fmt/format.h>

#include <fstream>
#include <streambuf>

namespace rawrbox {
	nlohmann::json Settings::getDefaults() {
		throw std::runtime_error("[RawrBox-Settings] Implement getDefaults");
	}

	const std::string Settings::getVersion() const {
		throw std::runtime_error("[RawrBox-Settings] Implement getVersion");
	}

	const std::string Settings::getFileName() const {
		throw std::runtime_error("[RawrBox-Settings] Implement getFileName");
	}

	void Settings::save() {
		auto& fileName = this->getFileName();

		std::ofstream out(fileName);
		if (!out.is_open()) throw std::runtime_error(fmt::format("[RawrBox-Settings] Failed to save settings '{}'", fileName));

		out << this->_settings.dump(1, '\t', false);
		out.close();
	}

	void Settings::load() {
		std::ifstream settingsFile(this->getFileName());
		std::string settingsStr((std::istreambuf_iterator<char>(settingsFile)), std::istreambuf_iterator<char>());

		this->_settings = nlohmann::json::parse(!settingsStr.empty() ? settingsStr : "{}", nullptr, true, true);

		if (this->_settings.empty()) {
			this->_settings = this->getDefaults();
			this->_settings["VERSION"] = this->getVersion(); // For settings version comparison

			this->save();
		} else {
			// Validate version
			auto version = _settings.find("VERSION");
			if (version == this->_settings.end() || version.value() != this->getVersion()) {
				fmt::print("[RawrBox-Settings] Migrating settings to version '{}'\n", this->getVersion());

				auto diff = nlohmann::json::diff(this->_settings, getDefaults());
				auto fixedDiff = nlohmann::json::array();

				for (auto& itm : diff) {
					if (itm["op"] == "replace" && itm["path"] != "/VERSION") continue;
					fixedDiff.push_back(itm);
				}

				this->_settings = this->_settings.patch(fixedDiff);
				this->_settings["VERSION"] = this->getVersion(); // For settings version comparison
				this->save();
			}
		}
	}

	nlohmann::json& Settings::getSettings() {
		return this->_settings;
	}
} // namespace rawrbox
