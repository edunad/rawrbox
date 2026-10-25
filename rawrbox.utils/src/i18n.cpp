
#include <rawrbox/utils/i18n.hpp>
#include <rawrbox/utils/vfs.hpp>

#include <glaze/glaze.hpp>

#include <fmt/args.h>
#include <fmt/printf.h>

#include <algorithm>
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

	void I18N::loadLanguagePack(const std::string& id, const std::filesystem::path& path) {
		_logger->info("Loading i18n for id {}", fmt::styled(id, fmt::fg(fmt::color::coral)));

		for (const auto& filePath : rawrbox::VFS::list(path)) {
			auto fileCleanName = filePath.stem().string();

			const auto data = rawrbox::VFS::read(filePath);
			std::string langRawStr(data.begin(), data.end());
			if (langRawStr.empty()) continue;

			try {
				rawrbox::Translation tr = {};
				if (glz::read_json(tr, langRawStr) != glz::error_code::none) RAWRBOX_CRITICAL("Invalid JSON file");

				addToLanguagePack(id, fileCleanName, tr);
				fmt::print("- '{}' language\n", fileCleanName);
			} catch (...) {
				_logger->warn("Invalid JSON file: {}", filePath.generic_string());
			}
		}
	}

	void I18N::setLanguage(const std::string& language) {
		_language = language;
	}

	const std::string& I18N::getLanguage() {
		return _language;
	}

	const rawrbox::Translation* I18N::findPack(const std::string& id) {
		auto pack = _languagePacks.find(fmt::format("{}-{}", id, _language));
		if (pack == _languagePacks.end()) {
			_logger->warn("Could not find language '{}' on '{}', falling back to english", _language, id);

			pack = _languagePacks.find(fmt::format("{}-en", id));
			if (pack == _languagePacks.end()) return nullptr;
		}

		return &pack->second;
	}

	std::string I18N::get(const std::string& id, const std::string& key) {
		const auto* pack = findPack(id);
		if (pack == nullptr) return fmt::format("$I18N FOR LANGUAGE '{}-{}' NOT FOUND$", id, _language);

		auto val = pack->find(key);
		if (val == pack->end()) return fmt::format("$I18N KEY '{}' NOT FOUND$", key);

		return val->second;
	}

	std::string I18N::get(const std::string& id, const std::string& key, const std::vector<std::string>& values) {
		fmt::dynamic_format_arg_store<fmt::format_context> args;
		for (const auto& str : values) {
			args.push_back(str);
		}

		return fmt::vformat(get(id, key), args);
	}

	std::optional<std::string> I18N::find(const std::string& id, const std::string& key) {
		const auto* pack = findPack(id);
		if (pack == nullptr) return std::nullopt;

		auto val = pack->find(key);
		if (val == pack->end()) return std::nullopt;

		return val->second;
	}

	std::optional<std::string> I18N::find(const std::string& id, const std::string& key, const std::vector<std::string>& values) {
		auto text = find(id, key);
		if (!text.has_value()) return std::nullopt;

		fmt::dynamic_format_arg_store<fmt::format_context> args;
		for (const auto& str : values) {
			args.push_back(str);
		}

		return fmt::vformat(*text, args);
	}

	std::optional<rawrbox::I18NKey> I18N::parseKey(std::string_view text) {
		if (text.size() < 2 || text.front() != '#') return std::nullopt;

		const auto body = text.substr(1);
		const bool valid = std::ranges::all_of(body, [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '-' || c == ':'; });
		if (!valid || std::ranges::count(body, ':') > 1) return std::nullopt;

		const auto colon = body.find(':');
		if (colon == std::string_view::npos) return rawrbox::I18NKey{.key = body};
		if (colon == 0 || colon == body.size() - 1) return std::nullopt;

		return rawrbox::I18NKey{.id = body.substr(0, colon), .key = body.substr(colon + 1)};
	}

	std::string I18N::resolve(std::string_view text, std::string_view id, const std::vector<std::string>& values) {
		const auto parsed = parseKey(text);
		if (!parsed.has_value()) return std::string(text);

		const std::string pack(parsed->id.empty() ? id : parsed->id);
		const std::string key(parsed->key);
		return values.empty() ? get(pack, key) : get(pack, key, values);
	}
} // namespace rawrbox
