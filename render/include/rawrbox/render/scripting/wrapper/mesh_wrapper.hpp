#pragma once

#include <rawrbox/render/model/mesh.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class MeshWrapper {
		std::shared_ptr<rawrbox::Mesh> ref;

	public:
		MeshWrapper(rawrbox::Mesh& ref);
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
