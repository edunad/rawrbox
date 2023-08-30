#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <unordered_map>

namespace rawrbox {
	using Language = std::unordered_map<std::string, nlohmann::json>;

	class I18N {
	private:
		static std::unordered_map<std::string, rawrbox::Language> _languagePacks;
		static std::string _language;

	public:
		static void initialize();
		static void addLanguagePack(const std::string& id, const std::string& lang, const nlohmann::json& json);
		static void loadLanguagePack(const std::string& id, const std::string& path);
		static void setLanguage(const std::string& language);
		static const std::string& getLanguage();

		static std::string get(const std::string& id, const std::string& key);
		static std::string get(const std::string& id, const std::string& key, std::vector<std::string> values);
	};
} // namespace rawrbox
