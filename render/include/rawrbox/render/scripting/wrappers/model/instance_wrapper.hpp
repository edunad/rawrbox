#pragma once
#include <rawrbox/render/model/instance.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class InstanceWrapper {
	public:
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
