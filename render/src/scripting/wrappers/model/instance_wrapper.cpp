#include <rawrbox/render/scripting/wrappers/model/instance_wrapper.hpp>

namespace rawrbox {
	void InstanceWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<Instance>("Instance",
		    sol::constructors<rawrbox::Instance(), rawrbox::Instance(rawrbox::Instance), rawrbox::Instance(const rawrbox::Matrix4x4&, const rawrbox::Colorf&, rawrbox::Vector4f)>(),

		    "getColor", &Instance::getColor,
		    "setColor", &Instance::setColor,

		    "getMatrix", &Instance::getMatrix,
		    "setMatrix", &Instance::setMatrix,

		    "getExtraData", &Instance::getExtraData,
		    "setExtraData", &Instance::setExtraData);
	}
	// -------------------------
} // namespace rawrbox
