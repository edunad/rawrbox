#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class Vector3Wrapper {
		template <typename T = int>
		static void registerTemplate(lua_State* L, const std::string& name) {
			using VEC = Vector3_t<T>;

			auto classDef = luabridge::getGlobalNamespace(L)
					    .beginClass<VEC>(name.c_str());

			classDef.addConstructor<void(), void(T), void(T, T, T), void(const std::array<T, 3>&), void(rawrbox::Vector2_t<T>, T)>()

			    .addProperty("x", &VEC::x)
			    .addProperty("y", &VEC::y)
			    .addProperty("y", &VEC::z)

			    .addStaticFunction("zero", &VEC::zero)
			    .addStaticFunction("one", &VEC::one)
			    .addStaticFunction("up", &VEC::up)
			    .addStaticFunction("forward", &VEC::forward)
			    .addStaticFunction("left", &VEC::left)

			    .addStaticFunction("mad",
				luabridge::overload<const VEC&, const T, const VEC&>(&VEC::mad),
				luabridge::overload<const VEC&, const VEC&, const VEC&>(&VEC::mad))

			    .addFunction("__tostring", &VEC::toString)
			    .addFunction("size", &VEC::size)

			    .addFunction("yxz", &VEC::yxz)
			    .addFunction("yzx", &VEC::yzx)
			    .addFunction("xzy", &VEC::xzy)
			    .addFunction("zxy", &VEC::zxy)
			    .addFunction("zyx", &VEC::zyx)

			    .addFunction("xy", &VEC::xy)
			    .addFunction("yx", &VEC::yx)
			    .addFunction("xz", &VEC::xz)
			    .addFunction("yz", &VEC::yz)
			    .addFunction("zx", &VEC::zx)
			    .addFunction("zy", &VEC::zy)

			    .addFunction("distance", &VEC::distance)
			    .addFunction("length", &VEC::length)
			    .addFunction("sqrMagnitude", &VEC::sqrMagnitude)
			    .addFunction("abs", &VEC::abs)

			    .addFunction("lerp", &VEC::lerp)

			    .addFunction("clamp",
				luabridge::overload<T, T>(&VEC::clamp),
				luabridge::overload<VEC, VEC>(&VEC::clamp))

			    .addFunction("clampMagnitude", &VEC::clampMagnitude)
			    .addFunction("min", &VEC::min)
			    .addFunction("max", &VEC::max)

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

			if constexpr (!std::is_same_v<VEC, Vector3_t<int>>) {
				classDef
				    .addFunction("angle", &VEC::angle)
				    .addFunction("rotateAroundOrigin", &VEC::rotateAroundOrigin)
				    .addFunction("normalized", &VEC::normalized)
				    .addFunction("dot", &VEC::dot)
				    .addFunction("floor", &VEC::floor)
				    .addFunction("round", &VEC::round)
				    .addFunction("ceil", &VEC::ceil)
				    .addFunction("cross", &VEC::cross)

				    .addStaticFunction("nan", &VEC::nan);
			}

			classDef.endClass();
		}

	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
