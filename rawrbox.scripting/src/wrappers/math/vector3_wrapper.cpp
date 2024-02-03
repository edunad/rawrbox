#include <rawrbox/math/vector3.hpp>
#include <rawrbox/scripting/wrappers/math/vector3_wrapper.hpp>

namespace rawrbox {
	void Vector3Wrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::Vector3>("Vector3")
		    .addConstructor<void(), void(rawrbox::Vector3), void(float), void(float, float, float), void(const std::array<float, 3>&), void(rawrbox::Vector2, float)>()

		    .addProperty("x", &rawrbox::Vector3::x)
		    .addProperty("y", &rawrbox::Vector3::y)
		    .addProperty("y", &rawrbox::Vector3::z)

		    .addStaticFunction("zero", &rawrbox::Vector3::zero)
		    .addStaticFunction("one", &rawrbox::Vector3::one)
		    .addStaticFunction("up", &rawrbox::Vector3::up)
		    .addStaticFunction("forward", &rawrbox::Vector3::forward)
		    .addStaticFunction("left", &rawrbox::Vector3::left)
		    .addStaticFunction("nan", &rawrbox::Vector3::nan)

		    .addStaticFunction("mad",
			luabridge::overload<const Vector3&, const float, const Vector3&>(&rawrbox::Vector3::mad),
			luabridge::overload<const Vector3&, const Vector3&, const Vector3&>(&rawrbox::Vector3::mad))

		    .addFunction("__tostring", &rawrbox::Vector3::toString)
		    .addFunction("size", &rawrbox::Vector3::size)

		    .addFunction("yxz", &rawrbox::Vector3::yxz)
		    .addFunction("yzx", &rawrbox::Vector3::yzx)
		    .addFunction("xzy", &rawrbox::Vector3::xzy)
		    .addFunction("zxy", &rawrbox::Vector3::zxy)
		    .addFunction("zyx", &rawrbox::Vector3::zyx)

		    .addFunction("xy", &rawrbox::Vector3::xy)
		    .addFunction("yx", &rawrbox::Vector3::yx)
		    .addFunction("xz", &rawrbox::Vector3::xz)
		    .addFunction("yz", &rawrbox::Vector3::yz)
		    .addFunction("zx", &rawrbox::Vector3::zx)
		    .addFunction("zy", &rawrbox::Vector3::zy)

		    .addFunction("distance", &rawrbox::Vector3::distance)
		    .addFunction("length", &rawrbox::Vector3::length)
		    .addFunction("sqrMagnitude", &rawrbox::Vector3::sqrMagnitude)
		    .addFunction("abs", &rawrbox::Vector3::abs)

		    .addFunction("angle", &rawrbox::Vector3::angle)
		    .addFunction("rotateAroundOrigin", &rawrbox::Vector3::rotateAroundOrigin)
		    .addFunction("lerp", &rawrbox::Vector3::lerp)

		    .addFunction("clamp",
			luabridge::overload<float, float>(&rawrbox::Vector3::clamp),
			luabridge::overload<Vector3, Vector3>(&rawrbox::Vector3::clamp))

		    .addFunction("clampMagnitude", &rawrbox::Vector3::clampMagnitude)

		    .addFunction("normalized", &rawrbox::Vector3::normalized)
		    .addFunction("dot", &rawrbox::Vector3::dot)
		    .addFunction("floor", &rawrbox::Vector3::floor)
		    .addFunction("round", &rawrbox::Vector3::round)
		    .addFunction("ceil", &rawrbox::Vector3::ceil)
		    .addFunction("cross", &rawrbox::Vector3::cross)
		    .addFunction("min", &rawrbox::Vector3::min)
		    .addFunction("max", &rawrbox::Vector3::max)

		    .addFunction("__add",
			luabridge::overload<float>(&rawrbox::Vector3::operator+),
			luabridge::overload<const Vector3&>(&rawrbox::Vector3::operator+))

		    .addFunction("__sub",
			luabridge::overload<float>(&rawrbox::Vector3::operator-),
			luabridge::overload<const Vector3&>(&rawrbox::Vector3::operator-))

		    .addFunction("__mul",
			luabridge::overload<float>(&rawrbox::Vector3::operator*),
			luabridge::overload<const Vector3&>(&rawrbox::Vector3::operator*))

		    .addFunction("__div",
			luabridge::overload<float>(&rawrbox::Vector3::operator/),
			luabridge::overload<const Vector3&>(&rawrbox::Vector3::operator/))

		    .addFunction("__eq",
			luabridge::overload<float>(&rawrbox::Vector3::operator==),
			luabridge::overload<const Vector3&>(&rawrbox::Vector3::operator==))

		    .addFunction("__ne",
			luabridge::overload<float>(&rawrbox::Vector3::operator!=),
			luabridge::overload<const Vector3&>(&rawrbox::Vector3::operator!=))

		    .addFunction("__lt",
			luabridge::overload<float>(&rawrbox::Vector3::operator<),
			luabridge::overload<const Vector3&>(&rawrbox::Vector3::operator<))

		    .addFunction("__le",
			luabridge::overload<float>(&rawrbox::Vector3::operator<=),
			luabridge::overload<const Vector3&>(&rawrbox::Vector3::operator<=))

		    .addFunction("__gt",
			luabridge::overload<float>(&rawrbox::Vector3::operator>),
			luabridge::overload<const Vector3&>(&rawrbox::Vector3::operator>))

		    .addFunction("__ge",
			luabridge::overload<float>(&rawrbox::Vector3::operator>=),
			luabridge::overload<const Vector3&>(&rawrbox::Vector3::operator>=))

		    .addFunction("__unm", [](rawrbox::Vector3& v) { return -v; })

		    .endClass();
	}
} // namespace rawrbox
