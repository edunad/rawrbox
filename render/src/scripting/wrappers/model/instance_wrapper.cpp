#include <rawrbox/render/scripting/wrappers/model/instance_wrapper.hpp>

namespace rawrbox {
	InstanceWrapper::InstanceWrapper(const rawrbox::Instance& ref_) : ref(ref_) {}

	rawrbox::Colori InstanceWrapper::getColor() { return ref.color.cast<int>(); }
	void InstanceWrapper::setColor(const rawrbox::Colori& color) { ref.color = color.cast<float>(); }

	const rawrbox::Matrix4x4& InstanceWrapper::getMatrix() { return ref.matrix; }
	void InstanceWrapper::setMatrix(const rawrbox::Matrix4x4& mtrx) { ref.matrix = mtrx; }

	const rawrbox::Vector4f& InstanceWrapper::getExtraData() { return ref.extraData; }
	void InstanceWrapper::setExtraData(const rawrbox::Vector4f& data) { ref.extraData = data; }

	void InstanceWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<InstanceWrapper>("Instance",
		    sol::constructors<rawrbox::Instance(), rawrbox::Instance(rawrbox::Instance)>(),

		    "getColor", &InstanceWrapper::getColor,
		    "setColor", &InstanceWrapper::setColor,

		    "getMatrix", &InstanceWrapper::getMatrix,
		    "setMatrix", &InstanceWrapper::setMatrix,

		    "getExtraData", &InstanceWrapper::getExtraData,
		    "setExtraData", &InstanceWrapper::setExtraData);
	}
	// -------------------------
} // namespace rawrbox
