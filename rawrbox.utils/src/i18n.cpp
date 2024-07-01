
#include <rawrbox/utils/i18n.hpp>

#include <glaze/glaze.hpp>

#include <fmt/args.h>
#include <fmt/printf.h>

#include <filesystem>
#include <fstream>

namespace rawrbox {
	// PRIVATE ----
	rawrbox::Language I18N::_languagePacks = {};
	std::string I18N::_language = "en"; // Default is ENGLISH

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> I18N::_logger = std::make_unique<rawrbox::Logger>("RawrBox-I18N");
	// -------------
	// ---------------

	void I18N::initialize() {
		loadLanguagePack("c++", "./i18n");
	}

	void I18N::addToLanguagePack(const std::string& id, const std::string& lang, const rawrbox::Translation& translation) {
		std::string packID = fmt::format("{}-{}", id, lang);

		auto pack = _languagePacks.find(packID);
		if (pack == _languagePacks.end()) {
			_languagePacks[packID] = translation;
		} else {
			_languagePacks[packID].insert(translation.begin(), translation.end());
		}
	}

	void I18N::loadLanguagePack(const std::string& id, const std::string& path) {
		_logger->info("Loading i18n for id {}", fmt::styled(id, fmt::fg(fmt::color::coral)));

		for (const auto& p : std::filesystem::directory_iterator(path)) {
			if (p.is_directory()) continue;

			auto fileName = p.path().filename().string();
			auto fileCleanName = p.path().stem().string();
			auto filePath = fmt::format("{}/{}", path, fileName);

			std::ifstream langRaw(filePath);
			std::string langRawStr((std::istreambuf_iterator<char>(langRaw)), std::istreambuf_iterator<char>());
			if (langRawStr.empty()) continue;

			try {
				rawrbox::Translation tr = {};
				if (glz::read_json(tr, langRawStr) != glz::error_code::none) throw _logger->error("Invalid JSON file");

				addToLanguagePack(id, fileCleanName, tr);
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
		auto pack = _languagePacks.find(fmt::format("{}-{}", id, _language));
		if (pack == _languagePacks.end()) {
			_logger->warn("Could not find language '{}' on '{}', falling back to english", _language, id);

			pack = _languagePacks.find(fmt::format("{}-en", id));
			if (pack == _languagePacks.end()) return fmt::format("$I18N FOR LANGUAGE '{}-{}' NOT FOUND$", id, _language); // Still no language? Bah..
		}

		auto val = pack->second.find(key);
		if (val == pack->second.end()) return fmt::format("$I18N KEY '{}' NOT FOUND$", key);

		return val->second;
	}

	std::string I18N::get(const std::string& id, const std::string& key, const std::vector<std::string>& values) {
		fmt::dynamic_format_arg_store<fmt::format_context> args;
		for (const auto& str : values) {
			args.push_back(str);
		}

		return fmt::vformat(get(id, key), args);
	}
} // namespace rawrbox
