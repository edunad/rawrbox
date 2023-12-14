#pragma once

#include <sol/sol.hpp>

namespace rawrbox {
	class ColorWrapper {
	public:
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
