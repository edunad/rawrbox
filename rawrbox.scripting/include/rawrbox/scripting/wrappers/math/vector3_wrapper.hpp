#pragma once

#include <sol/sol.hpp>

namespace rawrbox {
	class Vector3Wrapper {
	public:
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
