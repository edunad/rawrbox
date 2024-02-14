#pragma once
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class Vector2Wrapper {
		template <typename T = int>
		static void registerTemplate(lua_State* L, const std::string& name) {
			using VEC = Vector2_t<T>;

			auto classDef = luabridge::getGlobalNamespace(L)
					    .beginClass<VEC>(name.c_str());

			classDef.addConstructor<void(), void(T), void(T, T), void(std::array<T, 2>)>()

			    .addProperty("x", &VEC::x)
			    .addProperty("y", &VEC::y)

			    .addStaticFunction("zero", &VEC::zero)
			    .addStaticFunction("one", &VEC::one)

			    .addFunction("__tostring", &VEC::toString)
			    .addFunction("size", &VEC::size)
			    .addFunction("yx", &VEC::yx)
			    .addFunction("distance", &VEC::distance)
			    .addFunction("length", &VEC::length)
			    .addFunction("angle", &VEC::angle)
			    .addFunction("abs", &VEC::abs)

			    .addFunction("lerp", &VEC::lerp)
			    .addFunction("clamp",
				luabridge::overload<T, T>(&VEC::clamp),
				luabridge::overload<VEC, VEC>(&VEC::clamp))

			    .addFunction("sqrMagnitude", &VEC::sqrMagnitude)
			    .addFunction("clampMagnitude", &VEC::clampMagnitude)
			    .addFunction("min", &VEC::min)
			    .addFunction("max", &VEC::max)
			    .addFunction("atan2", &VEC::atan2)
			    .addFunction("dot", &VEC::dot)

			    .addFunction("intersects", &VEC::intersects)

			    .addFunction("__add",
				luabridge::overload<T>(&VEC::operator+),
				luabridge::overload<const VEC&>(&VEC::operator+))

			    .addFunction("__sub",
				luabridge::overload<T>(&VEC::operator-),
				luabridge::overload<const VEC&>(&VEC::operator-))

			    .addFunction("__mul",
				luabridge::overload<T>(&VEC::operator*),
				luabridge::overload<const VEC&>(&VEC::operator*))

			    .addFunction("__div",
				luabridge::overload<T>(&VEC::operator/),
				luabridge::overload<const VEC&>(&VEC::operator/))

			    .addFunction("__eq",
				luabridge::overload<T>(&VEC::operator==),
				luabridge::overload<const VEC&>(&VEC::operator==))

			    .addFunction("__ne",
				luabridge::overload<T>(&VEC::operator!=),
				luabridge::overload<const VEC&>(&VEC::operator!=))

			    .addFunction("__lt",
				luabridge::overload<T>(&VEC::operator<),
				luabridge::overload<const VEC&>(&VEC::operator<))

			    .addFunction("__le",
				luabridge::overload<T>(&VEC::operator<=),
				luabridge::overload<const VEC&>(&VEC::operator<=))

			    .addFunction("__gt",
				luabridge::overload<T>(&VEC::operator>),
				luabridge::overload<const VEC&>(&VEC::operator>))

			    .addFunction("__ge",
				luabridge::overload<T>(&VEC::operator>=),
				luabridge::overload<const VEC&>(&VEC::operator>=))

			    .addFunction("__unm", [](VEC& v) { return -v; });

			if constexpr (!std::is_same_v<VEC, Vector2_t<int>>) {
				classDef.addStaticFunction("sinCos", &VEC::sinCos)
				    .addStaticFunction("cosSin", &VEC::cosSin)

				    .addFunction("rotateAroundOrigin", &VEC::rotateAroundOrigin)
				    .addFunction("normalized", &VEC::normalized)
				    .addFunction("cross", &VEC::cross)
				    .addFunction("floor", &VEC::floor)
				    .addFunction("round", &VEC::round)
				    .addFunction("ceil", &VEC::ceil)
				    .addStaticFunction("nan", &VEC::nan);
			}

			classDef.endClass();
		}

	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
