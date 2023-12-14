#pragma once

#include <sol/sol.hpp>

namespace rawrbox {
	class FMTWrapper {
	public:
		std::string format(const std::string& str, sol::variadic_args va);
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
