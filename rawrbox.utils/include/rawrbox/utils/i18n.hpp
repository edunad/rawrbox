#pragma once

#include <rawrbox/utils/logger.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace rawrbox {
	using Translation = std::unordered_map<std::string, std::string>;
	using Language = std::unordered_map<std::string, rawrbox::Translation>;

	class I18N {
	private:
		static rawrbox::Language _languagePacks;
		static std::string _language;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

	public:
		static void initialize();
		static void addToLanguagePack(const std::string& id, const std::string& lang, const rawrbox::Translation& translation);
		static void loadLanguagePack(const std::string& id, const std::string& path);
		static void setLanguage(const std::string& language);
		static const std::string& getLanguage();

		static std::string get(const std::string& id, const std::string& key);
		static std::string get(const std::string& id, const std::string& key, const std::vector<std::string>& values);
	};
} // namespace rawrbox
