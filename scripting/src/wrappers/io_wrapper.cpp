
#include <rawrbox/scripting/wrappers/io_wrapper.hpp>

#include <fmt/format.h>

#include <filesystem>
#include <fstream>
#include <streambuf>

namespace rawrbox {
	std::pair<std::string, std::string> IOWrapper::load(const std::string& path) {
		if (!std::filesystem::exists("./data/" + path) || path.find("../") != std::string::npos) {
			return std::make_pair(fmt::format("Failed to find file {{data/{}}}", path), "");
		}

		std::ifstream filehandle("./data/" + path);
		if (!filehandle.is_open()) {
			return std::make_pair(fmt::format("Failed to open file {{data/{}}}", path), "");
		}

		std::stringstream dataStr;
		dataStr << filehandle.rdbuf();

		return std::make_pair("", dataStr.str());
	}

	std::string IOWrapper::save(const std::string& path, std::string data) {
		// Anti-d3lta 2000
		if (path.find("../") != std::string::npos) {
			return fmt::format("Failed to save file {{data/{}}}", path);
		}

		std::ofstream out(fmt::format("./data/{}", path));

		if (!out.is_open()) {
			return fmt::format("Failed to save file {{data/{}}}", path);
		}

		out << data;
		out.close();
		return "";
	}

	void IOWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::IOWrapper>("io",
		    sol::no_constructor,
		    "save", &rawrbox::IOWrapper::save,
		    "load", &rawrbox::IOWrapper::load);
	}
} // namespace rawrbox
