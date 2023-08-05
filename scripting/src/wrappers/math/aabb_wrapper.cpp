#include <rawrbox/math/aabb.hpp>
#include <rawrbox/scripting/wrappers/math/aabb_wrapper.hpp>

namespace rawrbox {
	void AABBWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::AABBf>("AABB",
		    sol::constructors<rawrbox::AABBf(), rawrbox::AABBf(rawrbox::AABBf), rawrbox::AABBf(float, float, float, float)>(),

		    "top", &AABBf::top,
		    "left", &AABBf::left,
		    "bottom", &AABBf::bottom,
		    "right", &AABBf::right,

		    "empty", &AABBf::empty,
		    "contains", &AABBf::contains,

		    "surfaceArea", &AABBf::surfaceArea
		    // sol::meta_function::equal_to, sol::overload(sol::resolve<rawrbox::AABBf(const rawrbox::AABBf&) const>(&rawrbox::AABBf::operator=))
		);
	}
} // namespace rawrbox
