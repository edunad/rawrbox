#pragma once

#include <rawrbox/utils/logger.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Luau {
	struct Frontend;
	struct FileResolver;
	struct ConfigResolver;
	struct Location;
} // namespace Luau

namespace rawrbox {
	// Uuses `.luaurc` next to the files / parent folders
	class LuaChecker {
	protected:
		struct VFSFiles;
		struct FolderConfig;

		std::unique_ptr<Luau::FileResolver> _files;
		std::unique_ptr<Luau::ConfigResolver> _config;
		std::unique_ptr<Luau::Frontend> _frontend;

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-LuaChecker");
		// -------------

		[[nodiscard]] static std::string readText(const std::filesystem::path& path);
		[[nodiscard]] static std::string problem(const std::string& file, const Luau::Location& location, std::string_view kind, std::string_view message);

	public:
		explicit LuaChecker(const std::vector<std::filesystem::path>& definitions = {});
		LuaChecker(const LuaChecker&) = delete;
		LuaChecker(LuaChecker&&) = delete;
		LuaChecker& operator=(const LuaChecker&) = delete;
		LuaChecker& operator=(LuaChecker&&) = delete;
		~LuaChecker();

		[[nodiscard]] std::vector<std::string> check(const std::filesystem::path& file) const;
		[[nodiscard]] std::vector<std::string> checkFolder(const std::filesystem::path& folder) const;
	};
} // namespace rawrbox