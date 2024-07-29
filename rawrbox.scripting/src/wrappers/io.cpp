
#include <rawrbox/scripting/wrappers/io.hpp>

#include <fmt/format.h>

#include <filesystem>
#include <fstream>

namespace rawrbox {
	std::pair<std::string, std::string> IOWrapper::load(const std::string& path) {
		std::filesystem::path dataDir = std::filesystem::absolute("./.data");
		std::filesystem::path filePath = dataDir / path;

		// Check if the resolved filePath is still within the data directory
		if (filePath.generic_string().find(dataDir.string()) != 0 || !std::filesystem::exists(filePath)) {
			return std::make_pair(fmt::format("File not found '{}'", path), "");
		}

		std::ifstream filehandle(filePath);
		if (!filehandle.is_open()) {
			return std::make_pair(fmt::format("Failed to open '{}'", path), "");
		}

		std::stringstream dataStr;
		dataStr << filehandle.rdbuf();

		return std::make_pair("", dataStr.str());
	}

	std::string IOWrapper::save(const std::string& path, const std::string& data) {
		std::filesystem::path dataDir = std::filesystem::absolute("./.data");
		std::filesystem::path filePath = dataDir / path;

		// Check if the resolved filePath is still within the data directory
		if (filePath.generic_string().find(dataDir.string()) != 0) {
			return fmt::format("File '{}' not found", path);
		}

		// Open the file for writing
		std::ofstream out(filePath, std::ios::binary); // Use binary mode to avoid issues with newline conversions
		if (!out.is_open()) {
			return fmt::format("Failed to create file '{}'", path);
		}

		// Write the data to the file
		out << data;
		if (!out.good()) {
			return fmt::format("Failed to write data to file '{}'", path);
		}

		return ""; // no error
	}

	void IOWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("io", {})
		    .addFunction("save", &rawrbox::IOWrapper::save)
		    .addFunction("load", &rawrbox::IOWrapper::load)
		    .endNamespace();
	}
} // namespace rawrbox
