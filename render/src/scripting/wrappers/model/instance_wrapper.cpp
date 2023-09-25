#include <rawrbox/render/model/instance.hpp>
#include <rawrbox/render/scripting/wrappers/model/instance_wrapper.hpp>

namespace rawrbox {
	void InstanceWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<Instance>("Instance",
		    sol::constructors<rawrbox::Instance(), rawrbox::Instance(rawrbox::Instance), rawrbox::Instance(const rawrbox::Matrix4x4&, const rawrbox::Colorf&, uint16_t, uint32_t)>(),

		    "getColor", &Instance::getColor,
		    "setColor", &Instance::setColor,

		    "getMatrix", &Instance::getMatrix,
		    "setMatrix", &Instance::setMatrix,

		    "getAtlasId", &Instance::getAtlasId,
		    "setAtlasId", &Instance::setAtlasId,

		    "getId", &Instance::getId,
		    "setId", &Instance::setId);
	}
	// -------------------------
} // namespace rawrbox
