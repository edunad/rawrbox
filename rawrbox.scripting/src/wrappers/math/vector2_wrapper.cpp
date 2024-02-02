#include <rawrbox/math/vector2.hpp>
#include <rawrbox/scripting/wrappers/math/vector2_wrapper.hpp>

namespace rawrbox {
	void Vector2Wrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::Vector2>("Vector2")
		    .addConstructor<void(), void(rawrbox::Vector2), void(float), void(float, float), void(std::array<float, 2>)>()

		    .addProperty("x", &rawrbox::Vector2::x)
		    .addProperty("y", &rawrbox::Vector2::y)

		    .addStaticFunction("zero", &rawrbox::Vector2::zero)
		    .addStaticFunction("one", &rawrbox::Vector2::one)
		    .addStaticFunction("nan", &rawrbox::Vector2::nan)

		    .addFunction("__tostring", &rawrbox::Vector2::toString)
		    .addFunction("size", &rawrbox::Vector2::size)
		    .addFunction("yx", &rawrbox::Vector2::yx)
		    .addFunction("distance", &rawrbox::Vector2::distance)
		    .addFunction("length", &rawrbox::Vector2::length)
		    .addFunction("angle", &rawrbox::Vector2::angle)
		    .addFunction("abs", &rawrbox::Vector2::abs)

		    .addFunction("lerp", &rawrbox::Vector2::lerp)
		    .addFunction("clamp",
			luabridge::overload<float, float>(&rawrbox::Vector2::clamp),
			luabridge::overload<Vector2, Vector2>(&rawrbox::Vector2::clamp))

		    .addFunction("clampMagnitude", &rawrbox::Vector2::clampMagnitude)
		    .addFunction("min", &rawrbox::Vector2::min)
		    .addFunction("max", &rawrbox::Vector2::max)
		    .addFunction("atan2", &rawrbox::Vector2::atan2)

		    .addStaticFunction("sinCos", &rawrbox::Vector2::sinCos)
		    .addStaticFunction("cosSin", &rawrbox::Vector2::cosSin)

		    .addFunction("intersects", &rawrbox::Vector2::intersects)
		    .addFunction("rotateAroundOrigin", &rawrbox::Vector2::rotateAroundOrigin)
		    .addFunction("dot", &rawrbox::Vector2::dot)
		    .addFunction("normalized", &rawrbox::Vector2::normalized)
		    .addFunction("cross", &rawrbox::Vector2::cross)
		    .addFunction("floor", &rawrbox::Vector2::floor)
		    .addFunction("round", &rawrbox::Vector2::round)
		    .addFunction("ceil", &rawrbox::Vector2::ceil)

		    .addFunction("__add",
			luabridge::overload<float>(&rawrbox::Vector2::operator+),
			luabridge::overload<const Vector2&>(&rawrbox::Vector2::operator+))

		    .addFunction("__sub",
			luabridge::overload<float>(&rawrbox::Vector2::operator-),
			luabridge::overload<const Vector2&>(&rawrbox::Vector2::operator-))

		    .addFunction("__mul",
			luabridge::overload<float>(&rawrbox::Vector2::operator*),
			luabridge::overload<const Vector2&>(&rawrbox::Vector2::operator*))

		    .addFunction("__div",
			luabridge::overload<float>(&rawrbox::Vector2::operator/),
			luabridge::overload<const Vector2&>(&rawrbox::Vector2::operator/))

		    .addFunction("__eq",
			luabridge::overload<float>(&rawrbox::Vector2::operator==),
			luabridge::overload<const Vector2&>(&rawrbox::Vector2::operator==))

		    .addFunction("__ne",
			luabridge::overload<float>(&rawrbox::Vector2::operator!=),
			luabridge::overload<const Vector2&>(&rawrbox::Vector2::operator!=))

		    .addFunction("__lt",
			luabridge::overload<float>(&rawrbox::Vector2::operator<),
			luabridge::overload<const Vector2&>(&rawrbox::Vector2::operator<))

		    .addFunction("__le",
			luabridge::overload<float>(&rawrbox::Vector2::operator<=),
			luabridge::overload<const Vector2&>(&rawrbox::Vector2::operator<=))

		    .addFunction("__gt",
			luabridge::overload<float>(&rawrbox::Vector2::operator>),
			luabridge::overload<const Vector2&>(&rawrbox::Vector2::operator>))

		    .addFunction("__ge",
			luabridge::overload<float>(&rawrbox::Vector2::operator>=),
			luabridge::overload<const Vector2&>(&rawrbox::Vector2::operator>=))

		    .addFunction("__unm", [](rawrbox::Vector2& v) { return -v; })

		    .endClass();
	}
} // namespace rawrbox
