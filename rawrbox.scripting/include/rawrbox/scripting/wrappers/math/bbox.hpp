#pragma once
#include <rawrbox/math/bbox.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class BBOXWrapper {

		template <typename T = int>
			requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
		static void registerTemplate(lua_State* L, const std::string& name) {
			using BBOXT = rawrbox::BBOX_t<T>;

			luabridge::getGlobalNamespace(L)
			    .beginClass<BBOXT>(name.c_str())
			    .template addConstructor<void(), void(BBOXT), void(const rawrbox::Vector3_t<T>&, const rawrbox::Vector3_t<T>&, const rawrbox::Vector3_t<T>&)>()

			    .addProperty("min", &BBOXT::min)
			    .addProperty("max", &BBOXT::max)
			    .addProperty("size", &BBOXT::size)

			    .addFunction("isEmpty", &BBOXT::isEmpty)
			    .addFunction("combine", &BBOXT::combine)

			    .addFunction("__eq", &BBOXT::operator==)
			    .addFunction("__ne", &BBOXT::operator!=)

			    .endClass();
		}

	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
