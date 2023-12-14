#pragma once

#include <sol/sol.hpp>

namespace rawrbox {
	class AABBWrapper {
	public:
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
