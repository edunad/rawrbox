
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/scripting/wrappers/model/model_wrapper.hpp>

namespace rawrbox {
	ModelWrapper::ModelWrapper(const std::shared_ptr<rawrbox::ModelBase<rawrbox::MaterialBase>>& ref) : rawrbox::ModelBaseWrapper(ref) {}

	// ANIMS ---
	void ModelWrapper::playAnimation(const std::string& name, sol::optional<bool> loop, sol::optional<float> speed) {
		if (!this->isValid()) return;
		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		ptr.lock()->playAnimation(name, loop.value_or(true), speed.value_or(1.F));
	}

	void ModelWrapper::stopAnimation(const std::string& name) {
		if (!this->isValid()) return;
		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		ptr.lock()->stopAnimation(name);
	}
	// ---

	// UTILS ----
	void ModelWrapper::setOptimizable(bool optimize) {
		if (!this->isValid()) return;
		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		ptr.lock()->setOptimizable(optimize);
	}

	void ModelWrapper::optimize() {
		if (!this->isValid()) return;
		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		ptr.lock()->optimize();
	}

	void ModelWrapper::setPos(const rawrbox::Vector3f& pos) {
		if (!this->isValid()) return;
		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		ptr.lock()->setPos(pos);
	}

	void ModelWrapper::setAngle(const rawrbox::Vector4f& angle) {
		if (!this->isValid()) return;
		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		ptr.lock()->setAngle(angle);
	}

	void ModelWrapper::setEulerAngle(const rawrbox::Vector3f& angle) {
		if (!this->isValid()) return;
		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		ptr.lock()->setEulerAngle(angle);
	}

	void ModelWrapper::setScale(const rawrbox::Vector3f& size) {
		if (!this->isValid()) return;
		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		ptr.lock()->setScale(size);
	}

	const rawrbox::BBOX ModelWrapper::getBBOX() const {
		if (!this->isValid()) return {};
		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		return ptr.lock()->getBBOX();
	}

	size_t ModelWrapper::totalMeshes() const {
		if (!this->isValid()) return 0;
		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		return ptr.lock()->totalMeshes();
	}

	bool ModelWrapper::empty() const {
		if (!this->isValid()) return true;
		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		return ptr.lock()->empty();
	}

	void ModelWrapper::removeMeshByName(const std::string& id) {
		if (!this->isValid()) return;
		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		return ptr.lock()->removeMeshByName(id);
	}

	void ModelWrapper::removeMesh(size_t index) {
		if (!this->isValid()) return;
		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		return ptr.lock()->removeMesh(index);
	}

	rawrbox::Mesh* ModelWrapper::getMeshByName(const std::string& id) {
		if (!this->isValid()) return nullptr;

		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		auto mesh = ptr.lock()->getMeshByName(id);
		if (mesh == nullptr) return nullptr;

		return mesh;
	}

	rawrbox::Mesh* ModelWrapper::getMesh(sol::optional<size_t> id) {
		if (!this->isValid()) return nullptr;

		std::weak_ptr<rawrbox::Model<>> ptr = std::dynamic_pointer_cast<rawrbox::Model<>>(this->_ref.lock());
		auto mesh = ptr.lock()->getMesh(id.value_or(0));
		if (mesh == nullptr) return nullptr;

		return mesh;
	}
	// ------

	void ModelWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<ModelWrapper>("Model<>",
		    sol::no_constructor,

		    // UTILS ----
		    "setOptimizable", &ModelWrapper::setOptimizable,
		    "optimize", &ModelWrapper::optimize,

		    "getBBOX", &ModelWrapper::getBBOX,

		    "totalMeshes", &ModelWrapper::totalMeshes,
		    "empty", &ModelWrapper::empty,

		    "removeMeshByName", &ModelWrapper::removeMeshByName,
		    "removeMesh", &ModelWrapper::removeMesh,
		    // ----

		    "getMeshByName", &ModelWrapper::getMeshByName,
		    "getMesh", &ModelWrapper::getMesh,

		    // ANIMS ---
		    "playAnimation", &ModelWrapper::playAnimation,
		    "stopAnimation", &ModelWrapper::stopAnimation,
		    // ----

		    sol::base_classes, sol::bases<rawrbox::ModelBaseWrapper>());
	}
} // namespace rawrbox
