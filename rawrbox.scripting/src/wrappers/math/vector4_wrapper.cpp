#include <rawrbox/math/vector4.hpp>
#include <rawrbox/scripting/wrappers/math/vector4_wrapper.hpp>

namespace rawrbox {
	void Vector4Wrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::Vector4>("Vector4")

		    .addConstructor<void(), void(rawrbox::Vector4), void(float), void(float, float, float, float), void(const std::array<float, 4>&), void(rawrbox::Vector3, float), void(rawrbox::Vector2, float, float)>()

		    .addProperty("x", &rawrbox::Vector4::x)
		    .addProperty("y", &rawrbox::Vector4::y)
		    .addProperty("y", &rawrbox::Vector4::z)
		    .addProperty("w", &rawrbox::Vector4::w)

		    .addStaticFunction("zero", &rawrbox::Vector4::zero)
		    .addStaticFunction("one", &rawrbox::Vector4::one)

		    .addFunction("size", &rawrbox::Vector4::size)
		    .addFunction("__tostring", &rawrbox::Vector4::toString)

		    .addFunction("yxz", &rawrbox::Vector4::yxz)
		    .addFunction("yzx", &rawrbox::Vector4::yzx)
		    .addFunction("xzy", &rawrbox::Vector4::xzy)
		    .addFunction("zxy", &rawrbox::Vector4::zxy)
		    .addFunction("zyx", &rawrbox::Vector4::zyx)

		    .addFunction("xy", &rawrbox::Vector4::xy)
		    .addFunction("yx", &rawrbox::Vector4::yx)
		    .addFunction("xz", &rawrbox::Vector4::xz)
		    .addFunction("yz", &rawrbox::Vector4::yz)
		    .addFunction("zx", &rawrbox::Vector4::zx)
		    .addFunction("zy", &rawrbox::Vector4::zy)

		    .addFunction("length", &rawrbox::Vector4::length)
		    .addFunction("sqrMagnitude", &rawrbox::Vector4::sqrMagnitude)
		    .addFunction("normalized", &rawrbox::Vector4::normalized)

		    .addFunction("clamp",
			luabridge::overload<float, float>(&rawrbox::Vector4::clamp),
			luabridge::overload<Vector4, Vector4>(&rawrbox::Vector4::clamp))

		    .addFunction("lerp", &rawrbox::Vector4::lerp)
		    .addFunction("toEuler", &rawrbox::Vector4::toEuler)
		    .addFunction("toAxis", &rawrbox::Vector4::toAxis)

		    .addStaticFunction("toQuat", &rawrbox::Vector4::toQuat)
		    .addStaticFunction("lookRotation", &rawrbox::Vector4::lookRotation)

		    .addFunction("inverse", &rawrbox::Vector4::inverse)
		    .addFunction("interpolate", &rawrbox::Vector4::interpolate)
		    .addFunction("min", &rawrbox::Vector4::min)
		    .addFunction("max", &rawrbox::Vector4::max)

		    .addFunction("__add",
			luabridge::overload<float>(&rawrbox::Vector4::operator+),
			luabridge::overload<const Vector4&>(&rawrbox::Vector4::operator+))

		    .addFunction("__sub",
			luabridge::overload<float>(&rawrbox::Vector4::operator-),
			luabridge::overload<const Vector4&>(&rawrbox::Vector4::operator-))

		    .addFunction("__mul",
			luabridge::overload<float>(&rawrbox::Vector4::operator*),
			luabridge::overload<const Vector4&>(&rawrbox::Vector4::operator*))

		    .addFunction("__div",
			luabridge::overload<float>(&rawrbox::Vector4::operator/),
			luabridge::overload<const Vector4&>(&rawrbox::Vector4::operator/))

		    .addFunction("__eq",
			luabridge::overload<float>(&rawrbox::Vector4::operator==),
			luabridge::overload<const Vector4&>(&rawrbox::Vector4::operator==))

		    .addFunction("__ne",
			luabridge::overload<float>(&rawrbox::Vector4::operator!=),
			luabridge::overload<const Vector4&>(&rawrbox::Vector4::operator!=))

		    .addFunction("__lt",
			luabridge::overload<float>(&rawrbox::Vector4::operator<),
			luabridge::overload<const Vector4&>(&rawrbox::Vector4::operator<))

		    .addFunction("__le",
			luabridge::overload<float>(&rawrbox::Vector4::operator<=),
			luabridge::overload<const Vector4&>(&rawrbox::Vector4::operator<=))

		    .addFunction("__gt",
			luabridge::overload<float>(&rawrbox::Vector4::operator>),
			luabridge::overload<const Vector4&>(&rawrbox::Vector4::operator>))

		    .addFunction("__ge",
			luabridge::overload<float>(&rawrbox::Vector4::operator>=),
			luabridge::overload<const Vector4&>(&rawrbox::Vector4::operator>=))

		    .addFunction("__unm", [](rawrbox::Vector4& v) { return -v; })

		    .endClass();
	}
} // namespace rawrbox
