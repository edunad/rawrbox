#include <rawrbox/math/vector4.hpp>
#include <rawrbox/scripting/wrappers/math/vector4_wrapper.hpp>

namespace rawrbox {
	void Vector4Wrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::Vector4f>("Vector4",
		    sol::constructors<rawrbox::Vector4f(), rawrbox::Vector3f(rawrbox::Vector4f), rawrbox::Vector4f(float), rawrbox::Vector4f(rawrbox::Vector3f, float), rawrbox::Vector4f(rawrbox::Vector2f, float, float), rawrbox::Vector4f(float, float, float, float)>(),

		    "x", &Vector4f::x,
		    "y", &Vector4f::y,
		    "z", &Vector4f::z,
		    "w", &Vector4f::w,

		    "xyz", &Vector4f::xyz,
		    "yxz", &Vector4f::yxz,
		    "yzx", &Vector4f::yzx,
		    "xzy", &Vector4f::xzy,
		    "zxy", &Vector4f::zxy,
		    "zyx", &Vector4f::zyx,
		    "xy", &Vector4f::xy,
		    "yx", &Vector4f::yx,

		    "data", &Vector4f::data,

		    "length", &Vector4f::length,
		    "lengthSquared", &Vector4f::lengthSquared,
		    "normalized", &Vector4f::normalized,

		    "clamp", sol::overload(sol::resolve<Vector4f(float, float) const>(&Vector4f::clamp), sol::resolve<Vector4f(Vector4f, Vector4f) const>(&Vector4f::clamp)),

		    "lerp", &Vector4f::lerp,
		    "toEuler", &Vector4f::toEuler,
		    "toAxis", &Vector4f::toAxis,
		    "toQuat", &Vector4f::toQuat,
		    "lookRotation", &Vector4f::lookRotation,
		    "inverse", &Vector4f::inverse,
		    "interpolate", &Vector4f::interpolate,

		    sol::meta_function::less_than, sol::overload(sol::resolve<bool(const Vector4f&) const>(&Vector4f::operator<), sol::resolve<bool(float) const>(&Vector4f::operator<)),
		    sol::meta_function::less_than_or_equal_to, sol::overload(sol::resolve<bool(const Vector4f&) const>(&Vector4f::operator<=), sol::resolve<bool(float) const>(&Vector4f::operator<=)),
		    sol::meta_function::equal_to, sol::overload(sol::resolve<bool(const Vector4f&) const>(&Vector4f::operator==), sol::resolve<bool(float) const>(&Vector4f::operator==)),

		    sol::meta_function::addition, sol::overload(sol::resolve<Vector4f(const Vector4f&) const>(&Vector4f::operator+), sol::resolve<Vector4f(float) const>(&Vector4f::operator+)),
		    sol::meta_function::subtraction, sol::overload(sol::resolve<Vector4f(const Vector4f&) const>(&Vector4f::operator-), sol::resolve<Vector4f(float) const>(&Vector4f::operator-)),
		    sol::meta_function::division, sol::overload(sol::resolve<Vector4f(const Vector4f&) const>(&Vector4f::operator/), sol::resolve<Vector4f(float) const>(&Vector4f::operator/)),
		    sol::meta_function::multiplication, sol::overload(sol::resolve<Vector4f(const Vector4f&) const>(&Vector4f::operator*), sol::resolve<Vector4f(float) const>(&Vector4f::operator*)));
	}
} // namespace rawrbox
