
#include <rawrbox/utils/i18n.hpp>

#include <fmt/args.h>
#include <fmt/printf.h>

#include <filesystem>
#include <fstream>

namespace rawrbox {
	// PRIVATE ----
	std::unordered_map<std::string, rawrbox::Language> I18N::_languagePacks = {};
	std::string I18N::_language = "en"; // Default is ENGLISH

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> I18N::_logger = std::make_unique<rawrbox::Logger>("RawrBox-I18N");
	// -------------
	// ---------------

	void I18N::initialize() {
		loadLanguagePack("c++", "./i18n");
	}

	void I18N::addLanguagePack(const std::string& id, const std::string& lang, const nlohmann::json& json) {
		auto pack = _languagePacks.find(id);
		if (pack == _languagePacks.end()) pack = _languagePacks.emplace(id, Language{}).first;

		pack->second[lang] = json;
	}

	void I18N::loadLanguagePack(const std::string& id, const std::string& path) {
		_logger->info("Loading i18n for id {}", fmt::format(fmt::fg(fmt::color::coral), id));

		for (auto& p : std::filesystem::directory_iterator(path)) {
			if (p.is_directory()) continue;

			auto fileName = p.path().filename().string();
			auto fileCleanName = p.path().stem().string();
			auto filePath = fmt::format("{}/{}", path, fileName);

			std::ifstream langRaw(filePath);
			std::string langRawStr((std::istreambuf_iterator<char>(langRaw)), std::istreambuf_iterator<char>());
			if (langRawStr.empty()) continue;

			try {
				addLanguagePack(id, fileCleanName, nlohmann::json::parse(langRawStr));
				fmt::print("- '{}' language\n", fileCleanName);
			} catch (...) {
				_logger->warn("Invalid JSON file: {}", filePath);
			}
		}
	}

	void I18N::setLanguage(const std::string& language) {
		_language = language;
	}

	const std::string& I18N::getLanguage() {
		return _language;
	}

	std::string I18N::get(const std::string& id, const std::string& key) {
		auto pack = _languagePacks.find(id);
		if (pack == _languagePacks.end()) return fmt::format("$I18N FOR ID '{}' NOT FOUND$", id);

		auto lang = pack->second.find(_language);
		if (lang == pack->second.end()) {
			_logger->warn("Could not find language '[#ffffff]{}[/]' on '[#ffffff]{}[/]', falling back to english\n", _language, id);

			lang = pack->second.find("en");
			if (lang == pack->second.end()) return fmt::format("$I18N FOR LANGUAGE '{}' NOT FOUND$", _language); // Still no language? Bah..
		}

		auto val = lang->second.find(key);
		if (val == lang->second.end()) return fmt::format("$I18N KEY '{}' NOT FOUND$", key);

		return val.value();
	}

	std::string I18N::get(const std::string& id, const std::string& key, std::vector<std::string> values) {
		fmt::dynamic_format_arg_store<fmt::format_context> args;
		for (std::string& str : values)
			args.push_back(str);

		return fmt::vformat(get(id, key), args);
	}
} // namespace rawrbox
