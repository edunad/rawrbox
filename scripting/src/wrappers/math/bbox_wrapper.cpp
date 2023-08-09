#include <rawrbox/math/bbox.hpp>
#include <rawrbox/scripting/wrappers/math/bbox_wrapper.hpp>

namespace rawrbox {
	void BBOXWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::BBOX>("BBOX",
		    sol::constructors<rawrbox::BBOX(), rawrbox::BBOX(rawrbox::BBOX), rawrbox::BBOXf(rawrbox::Vector3f, rawrbox::Vector3f, rawrbox::Vector3f)>(),
		    "combine", &BBOXf::combine,
		    sol::meta_function::equal_to, &rawrbox::BBOX::operator==);
	}
} // namespace rawrbox
