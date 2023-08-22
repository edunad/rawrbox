
#include <rawrbox/render/model/instanced.hpp>
#include <rawrbox/render/scripting/wrappers/model/instanced_wrapper.hpp>

namespace rawrbox {
	InstancedModelWrapper::InstancedModelWrapper(const std::shared_ptr<rawrbox::ModelBase>& ref) : rawrbox::ModelBaseWrapper(ref) {}

	// UTILS ----
	void InstancedModelWrapper::addInstance(const rawrbox::Instance& instance) {
		if (!this->isValid()) return;

		std::weak_ptr<rawrbox::InstancedModel> ptr = std::dynamic_pointer_cast<rawrbox::InstancedModel>(this->_ref.lock());
		ptr.lock()->addInstance(instance);
	}

	void InstancedModelWrapper::removeInstance(size_t i) {
		if (!this->isValid()) return;

		std::weak_ptr<rawrbox::InstancedModel> ptr = std::dynamic_pointer_cast<rawrbox::InstancedModel>(this->_ref.lock());
		ptr.lock()->removeInstance(i);
	}

	rawrbox::Instance& InstancedModelWrapper::getInstance(size_t i) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-InstancedModelWrapper] Invalid instanced model, reference is not set!");

		std::weak_ptr<rawrbox::InstancedModel> ptr = std::dynamic_pointer_cast<rawrbox::InstancedModel>(this->_ref.lock());
		return ptr.lock()->getInstance(i);
	}
	// --------

	size_t InstancedModelWrapper::count() const {
		if (!this->isValid()) return 0;

		std::weak_ptr<rawrbox::InstancedModel> ptr = std::dynamic_pointer_cast<rawrbox::InstancedModel>(this->_ref.lock());
		return ptr.lock()->count();
	}

	// UPLOAD ----
	void InstancedModelWrapper::setAutoUpload(bool set) {
		if (!this->isValid()) return;

		std::weak_ptr<rawrbox::InstancedModel> ptr = std::dynamic_pointer_cast<rawrbox::InstancedModel>(this->_ref.lock());
		ptr.lock()->setAutoUpload(set);
	}

	void InstancedModelWrapper::updateInstance() {
		if (!this->isValid()) return;

		std::weak_ptr<rawrbox::InstancedModel> ptr = std::dynamic_pointer_cast<rawrbox::InstancedModel>(this->_ref.lock());
		ptr.lock()->updateInstance();
	}
	// ----

	void InstancedModelWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<InstancedModelWrapper>("InstancedModel",
		    sol::no_constructor,

		    // UTILS ----
		    "addInstance", &InstancedModelWrapper::addInstance,
		    "removeInstance", &InstancedModelWrapper::removeInstance,
		    "getInstance", &InstancedModelWrapper::getInstance,
		    // ----

		    "count", &InstancedModelWrapper::count,

		    // UPLOAD ----
		    "updateInstance", &InstancedModelWrapper::updateInstance,
		    "setAutoUpload", &InstancedModelWrapper::setAutoUpload,
		    // ----

		    sol::base_classes, sol::bases<rawrbox::ModelBaseWrapper>());
	}
} // namespace rawrbox
