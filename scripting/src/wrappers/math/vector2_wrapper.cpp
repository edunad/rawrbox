#include <rawrbox/math/vector2.hpp>
#include <rawrbox/scripting/wrappers/math/vector2_wrapper.hpp>

namespace rawrbox {
	void Vector2Wrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::Vector2f>("Vector2",
		    sol::constructors<rawrbox::Vector2f(), rawrbox::Vector2f(rawrbox::Vector2f), rawrbox::Vector2f(float), rawrbox::Vector2f(float, float)>(),

		    "x", &Vector2f::x,
		    "y", &Vector2f::y,
		    "yx", &Vector2f::yx,

		    "zero", &Vector2f::zero,
		    "one", &Vector2f::one,
		    "size", &Vector2f::size,
		    "data", &Vector2f::data,

		    "clamp", sol::overload(sol::resolve<Vector2f(float, float) const>(&Vector2f::clamp), sol::resolve<Vector2f(Vector2f, Vector2f) const>(&Vector2f::clamp)),

		    "distance", &Vector2f::distance,
		    "length", &Vector2f::length,
		    "angle", &Vector2f::angle,
		    "abs", &Vector2f::abs,
		    "lerp", &Vector2f::lerp,
		    "atan2", &Vector2f::atan2,
		    "sinCos", &Vector2f::sinCos,
		    "cosSin", &Vector2f::cosSin,
		    "intersects", &Vector2f::intersects,
		    "rotateAroundOrigin", &Vector2f::rotateAroundOrigin,
		    "dot", &Vector2f::dot,
		    "normalized", &Vector2f::normalized,
		    "cross", &Vector2f::cross,
		    "floor", &Vector2f::floor,
		    "round", &Vector2f::round,
		    "ceil", &Vector2f::ceil,

		    sol::meta_function::less_than, sol::overload(sol::resolve<bool(const Vector2f&) const>(&Vector2f::operator<), sol::resolve<bool(float) const>(&Vector2f::operator<)),
		    sol::meta_function::less_than_or_equal_to, sol::overload(sol::resolve<bool(const Vector2f&) const>(&Vector2f::operator<=), sol::resolve<bool(float) const>(&Vector2f::operator<=)),
		    sol::meta_function::equal_to, sol::overload(sol::resolve<bool(const Vector2f&) const>(&Vector2f::operator==), sol::resolve<bool(float) const>(&Vector2f::operator==)),

		    sol::meta_function::addition, sol::overload(sol::resolve<Vector2f(const Vector2f&) const>(&Vector2f::operator+), sol::resolve<Vector2f(float) const>(&Vector2f::operator+)),
		    sol::meta_function::subtraction, sol::overload(sol::resolve<Vector2f(const Vector2f&) const>(&Vector2f::operator-), sol::resolve<Vector2f(float) const>(&Vector2f::operator-)),
		    sol::meta_function::division, sol::overload(sol::resolve<Vector2f(const Vector2f&) const>(&Vector2f::operator/), sol::resolve<Vector2f(float) const>(&Vector2f::operator/)),
		    sol::meta_function::multiplication, sol::overload(sol::resolve<Vector2f(const Vector2f&) const>(&Vector2f::operator*), sol::resolve<Vector2f(float) const>(&Vector2f::operator*)));
	}
} // namespace rawrbox
