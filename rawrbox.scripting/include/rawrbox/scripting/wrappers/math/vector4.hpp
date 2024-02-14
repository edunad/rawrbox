#pragma once

#include <rawrbox/math/vector4.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class Vector4Wrapper {
		template <typename T = int>
		static void registerTemplate(lua_State* L, const std::string& name) {
			using VEC = Vector4_t<T>;

			auto classDef = luabridge::getGlobalNamespace(L)
					    .beginClass<VEC>(name.c_str());

			classDef
			    .addConstructor<void(), void(T), void(T, T, T, T), void(const std::array<T, 4>&), void(rawrbox::Vector3_t<T>, T), void(rawrbox::Vector2_t<T>, T, T)>()
			    .addProperty("x", &VEC::x)
			    .addProperty("y", &VEC::y)
			    .addProperty("y", &VEC::z)
			    .addProperty("w", &VEC::w)

			    .addStaticFunction("zero", &VEC::zero)
			    .addStaticFunction("one", &VEC::one)

			    .addFunction("size", &VEC::size)
			    .addFunction("__tostring", &VEC::toString)

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

			    .addFunction("length", &VEC::length)
			    .addFunction("sqrMagnitude", &VEC::sqrMagnitude)
			    .addFunction("normalized", &VEC::normalized)

			    .addFunction("clamp",
				luabridge::overload<T, T>(&VEC::clamp),
				luabridge::overload<VEC, VEC>(&VEC::clamp))

			    .addFunction("lerp", &VEC::lerp)

			    .addFunction("inverse", &VEC::inverse)
			    .addFunction("interpolate", &VEC::interpolate)
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

			if constexpr (!std::is_same_v<VEC, Vector4_t<int>>) {
				classDef
				    .addFunction("toEuler", &VEC::toEuler)
				    .addFunction("toAxis", &VEC::toAxis)

				    .addStaticFunction("toQuat", &VEC::toQuat)
				    .addStaticFunction("lookRotation", &VEC::lookRotation)
				    .addStaticFunction("nan", &VEC::nan);
			}

			classDef.endClass();
		}

	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
