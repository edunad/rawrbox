
#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/scripting/wrappers/model/base_wrapper.hpp>

namespace rawrbox {
	ModelBaseWrapper::ModelBaseWrapper(const std::shared_ptr<rawrbox::ModelBase>& ref) : _ref(ref) {}
	ModelBaseWrapper::~ModelBaseWrapper() { this->_ref.reset(); }

	// UTILS ----
	const rawrbox::Vector3f ModelBaseWrapper::getPos() const {
		if (!this->isValid()) return {};
		return this->_ref.lock()->getPos();
	}

	void ModelBaseWrapper::setPos(const rawrbox::Vector3f& pos) {
		if (!this->isValid()) return;
		this->_ref.lock()->setPos(pos);
	}

	const rawrbox::Vector3f ModelBaseWrapper::getScale() const {
		if (!this->isValid()) return {};
		return this->_ref.lock()->getScale();
	}

	void ModelBaseWrapper::setScale(const rawrbox::Vector3f& scale) {
		if (!this->isValid()) return;
		this->_ref.lock()->setScale(scale);
	}

	const rawrbox::Vector4f ModelBaseWrapper::getAngle() const {
		if (!this->isValid()) return {};
		return this->_ref.lock()->getAngle();
	}

	void ModelBaseWrapper::setAngle(const rawrbox::Vector4f& ang) {
		if (!this->isValid()) return;
		this->_ref.lock()->setAngle(ang);
	}

	void ModelBaseWrapper::setEulerAngle(const rawrbox::Vector3f& ang) {
		if (!this->isValid()) return;
		this->_ref.lock()->setEulerAngle(ang);
	}

	const rawrbox::Matrix4x4 ModelBaseWrapper::getMatrix() const {
		if (!this->isValid()) return {};
		return this->_ref.lock()->getMatrix();
	}

	bool ModelBaseWrapper::isDynamic() const {
		if (!this->isValid()) return false;
		return this->_ref.lock()->isDynamic();
	}

	bool ModelBaseWrapper::isUploaded() const {
		if (!this->isValid()) return false;
		return this->_ref.lock()->isUploaded();
	}
	// ------

	bool ModelBaseWrapper::isValid() const {
		return !this->_ref.expired();
	}

	void ModelBaseWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<ModelBaseWrapper>("ModelBase",
		    sol::no_constructor,

		    // UTILS ----
		    "getPos", &ModelBaseWrapper::getPos,
		    "setPos", &ModelBaseWrapper::setPos,

		    "getScale", &ModelBaseWrapper::getScale,
		    "setScale", &ModelBaseWrapper::setScale,

		    "getAngle", &ModelBaseWrapper::getAngle,
		    "setAngle", &ModelBaseWrapper::setAngle,
		    "setEulerAngle", &ModelBaseWrapper::setEulerAngle,

		    "getMatrix", &ModelBaseWrapper::getMatrix,
		    "isDynamic", &ModelBaseWrapper::isDynamic,
		    "isUploaded", &ModelBaseWrapper::isUploaded,
		    // --------------

		    "isValid", &ModelBaseWrapper::isValid);
	}
} // namespace rawrbox
