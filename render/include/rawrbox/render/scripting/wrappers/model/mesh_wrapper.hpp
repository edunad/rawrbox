#pragma once

#include <rawrbox/render/model/mesh.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class MeshWrapper {
	public:
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
