#pragma once
#include <rawrbox/math/bbox.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

#include <vector>

namespace rawrbox {
	class BBOXWrapper {

		template <typename T>
			requires(std::is_floating_point_v<T>)
		static bool inPlanes(const rawrbox::BBOX_t<T>* self, const std::vector<rawrbox::Vector4_t<T>>& planes) {
			return self->inPlanes(planes);
		}

		template <typename T = int>
			requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
		static void registerTemplate(lua_State* L, const std::string& name) {
			using BBOXT = rawrbox::BBOX_t<T>;
			using VECT = rawrbox::Vector3_t<T>;

			luabridge::getGlobalNamespace(L)
			    .beginClass<BBOXT>(name.c_str())
			    .template addConstructor<void(), void(BBOXT), void(const rawrbox::Vector3_t<T>&, const rawrbox::Vector3_t<T>&), void(const rawrbox::Vector3_t<T>&, const rawrbox::Vector3_t<T>&, const rawrbox::Vector3_t<T>&)>()

			    .addProperty("min", &BBOXT::min)
			    .addProperty("max", &BBOXT::max)
			    .addProperty("size", &BBOXT::size)

			    .addFunction("isEmpty", &BBOXT::isEmpty)
			    .addFunction("contains", &BBOXT::contains)
			    .addFunction("combine", &BBOXT::combine)

			    .addFunction("__mul",
				luabridge::overload<T>(&BBOXT::operator*),
				luabridge::overload<const VECT&>(&BBOXT::operator*))

			    .addFunction("__eq", &BBOXT::operator==)
			    .addFunction("__ne", &BBOXT::operator!=)

			    .endClass();

			if constexpr (std::is_floating_point_v<T>) {
				luabridge::getGlobalNamespace(L)
				    .beginClass<BBOXT>(name.c_str())
				    .addFunction("inPlanes", &BBOXWrapper::inPlanes<T>)
				    .endClass();
			}
		}

	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox