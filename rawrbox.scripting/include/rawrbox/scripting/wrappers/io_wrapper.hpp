#pragma once

#include <sol/sol.hpp>

namespace rawrbox {
	class IOWrapper {
	public:
		std::pair<std::string, std::string> load(const std::string& path);
		std::string save(const std::string& path, std::string data);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
