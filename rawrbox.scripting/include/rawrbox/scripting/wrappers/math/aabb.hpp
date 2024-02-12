#pragma once

#include <rawrbox/math/aabb.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class AABBWrapper {
		template <typename T = int>
		static void registerTemplate(lua_State* L, const std::string& name) {
			using AABBT = rawrbox::AABB_t<T>;

			luabridge::getGlobalNamespace(L)
			    .beginClass<AABBT>(name.c_str())
			    .addConstructor<void(), void(AABBT), void(T, T, T, T), void(const Vector2_t<T>&, const Vector2_t<T>&)>()

			    .addProperty("position", &AABBT::pos)
			    .addProperty("size", &AABBT::size)

			    .addFunction("surfaceArea", &AABBT::surfaceArea)
			    .addFunction("top", &AABBT::top)
			    .addFunction("right", &AABBT::right)
			    .addFunction("left", &AABBT::left)
			    .addFunction("bottom", &AABBT::bottom)
			    .addFunction("empty", &AABBT::empty)

			    .addFunction("contains",
				luabridge::overload<const Vector2_t<T>&>(&AABBT::contains),
				luabridge::overload<const AABBT&>(&AABBT::contains))

			    .addFunction("intersects", &AABBT::intersects)
			    .addFunction("mask", &AABBT::mask)

			    .addFunction("__mul", &AABBT::operator*)
			    .addFunction("__eq", &AABBT::operator==)
			    .addFunction("__ne", &AABBT::operator!=)

			    .endClass();
		}

	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
