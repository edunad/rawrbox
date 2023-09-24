#include <rawrbox/math/vector3.hpp>
#include <rawrbox/scripting/wrappers/math/vector3_wrapper.hpp>

namespace rawrbox {
	void Vector3Wrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::Vector3f>("Vector3",
		    sol::constructors<rawrbox::Vector3f(), rawrbox::Vector3f(rawrbox::Vector3f), rawrbox::Vector3f(rawrbox::Vector2f, float), rawrbox::Vector3f(float), rawrbox::Vector3f(float, float), rawrbox::Vector3f(float, float, float)>(),

		    "x", &Vector3f::x,
		    "y", &Vector3f::y,
		    "z", &Vector3f::z,

		    "yxz", &Vector3f::yxz,
		    "yzx", &Vector3f::yzx,
		    "xzy", &Vector3f::xzy,
		    "zxy", &Vector3f::zxy,
		    "zyx", &Vector3f::zyx,
		    "xy", &Vector3f::xy,
		    "yx", &Vector3f::yx,

		    "data", &Vector3f::data,

		    "clamp", sol::overload(sol::resolve<Vector3f(float, float) const>(&Vector3f::clamp), sol::resolve<Vector3f(Vector3f, Vector3f) const>(&Vector3f::clamp)),

		    "distance", &Vector3f::distance,
		    "length", &Vector3f::length,
		    "sqrMagnitude", &Vector3f::sqrMagnitude,
		    "abs", &Vector3f::abs,
		    "angle", &Vector3f::angle,
		    "rotate", &Vector3f::rotate,
		    "lerp", &Vector3f::lerp,
		    "toArray", &Vector3f::toArray,
		    "normalized", &Vector3f::normalized,
		    "dot", &Vector3f::dot,
		    "floor", &Vector3f::floor,
		    "round", &Vector3f::round,
		    "ceil", &Vector3f::ceil,
		    "cross", &Vector3f::cross,

		    sol::meta_function::less_than, sol::overload(sol::resolve<bool(const Vector3f&) const>(&Vector3f::operator<), sol::resolve<bool(float) const>(&Vector3f::operator<)),
		    sol::meta_function::less_than_or_equal_to, sol::overload(sol::resolve<bool(const Vector3f&) const>(&Vector3f::operator<=), sol::resolve<bool(float) const>(&Vector3f::operator<=)),
		    sol::meta_function::equal_to, sol::overload(sol::resolve<bool(const Vector3f&) const>(&Vector3f::operator==), sol::resolve<bool(float) const>(&Vector3f::operator==)),

		    sol::meta_function::addition, sol::overload(sol::resolve<Vector3f(const Vector3f&) const>(&Vector3f::operator+), sol::resolve<Vector3f(float) const>(&Vector3f::operator+)),
		    sol::meta_function::subtraction, sol::overload(sol::resolve<Vector3f(const Vector3f&) const>(&Vector3f::operator-), sol::resolve<Vector3f(float) const>(&Vector3f::operator-)),
		    sol::meta_function::division, sol::overload(sol::resolve<Vector3f(const Vector3f&) const>(&Vector3f::operator/), sol::resolve<Vector3f(float) const>(&Vector3f::operator/)),
		    sol::meta_function::multiplication, sol::overload(sol::resolve<Vector3f(const Vector3f&) const>(&Vector3f::operator*), sol::resolve<Vector3f(float) const>(&Vector3f::operator*)));
	}
} // namespace rawrbox
