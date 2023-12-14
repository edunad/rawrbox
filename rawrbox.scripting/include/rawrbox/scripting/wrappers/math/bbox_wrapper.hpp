#pragma once

#include <sol/sol.hpp>

namespace rawrbox {
	class BBOXWrapper {
	public:
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
