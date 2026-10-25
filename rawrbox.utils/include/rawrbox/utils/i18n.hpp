#pragma once

#include <rawrbox/utils/logger.hpp>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace rawrbox {
	using Translation = std::unordered_map<std::string, std::string>;
	using Language = std::unordered_map<std::string, rawrbox::Translation>;

	struct I18NKey {
		std::string_view id = {};
		std::string_view key = {};
	};

	class I18N {
	private:
		static rawrbox::Language _languagePacks;
		static std::string _language;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

		static const rawrbox::Translation* findPack(const std::string& id);

	public:
		static void initialize();
		static void addToLanguagePack(const std::string& id, const std::string& lang, const rawrbox::Translation& translation);
		static void loadLanguagePack(const std::string& id, const std::filesystem::path& path);
		static void setLanguage(const std::string& language);
		static const std::string& getLanguage();

		static std::string get(const std::string& id, const std::string& key);
		static std::string get(const std::string& id, const std::string& key, const std::vector<std::string>& values);

		[[nodiscard]] static std::optional<std::string> find(const std::string& id, const std::string& key);
		[[nodiscard]] static std::optional<std::string> find(const std::string& id, const std::string& key, const std::vector<std::string>& values);

		[[nodiscard]] static std::optional<rawrbox::I18NKey> parseKey(std::string_view text);
		[[nodiscard]] static std::string resolve(std::string_view text, std::string_view id, const std::vector<std::string>& values = {});
	};
} // namespace rawrbox
