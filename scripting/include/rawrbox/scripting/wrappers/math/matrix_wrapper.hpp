#pragma once

#include <sol/sol.hpp>

namespace rawrbox {
	class MatrixWrapper {
	public:
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
