#pragma once

#include <sol/sol.hpp>

namespace rawrbox {
	class ConsoleCommandWrapper {
	public:
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
