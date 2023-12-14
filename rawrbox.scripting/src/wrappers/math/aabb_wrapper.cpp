#include <rawrbox/math/aabb.hpp>
#include <rawrbox/scripting/wrappers/math/aabb_wrapper.hpp>

namespace rawrbox {
	void AABBWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::AABB>("AABB",
		    sol::constructors<rawrbox::AABB(), rawrbox::AABB(rawrbox::AABB), rawrbox::AABB(float, float, float, float)>(),

		    "top", &AABB::top,
		    "left", &AABB::left,
		    "bottom", &AABB::bottom,
		    "right", &AABB::right,

		    "empty", &AABB::empty,
		    "contains", &AABB::contains,

		    "surfaceArea", &AABB::surfaceArea,

		    sol::meta_function::equal_to, &rawrbox::AABB::operator==,
		    sol::meta_function::multiplication, &rawrbox::AABB::operator*);
	}
} // namespace rawrbox
