#pragma once
#include <sol/sol.hpp>

namespace rawrbox {
	class GraphCatWrapper {
	public:
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
