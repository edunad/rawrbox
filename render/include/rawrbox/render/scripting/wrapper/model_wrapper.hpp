#pragma once

#include <rawrbox/render/model/model.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	template <typename M = rawrbox::MaterialBase>
	class ModelWrapper {
		std::weak_ptr<rawrbox::ModelBase<M>> _ref;

	public:
		ModelWrapper(rawrbox::ModelBase<M>& ref) : _ref(ref.weak_from_this()) {}
		ModelWrapper(const ModelWrapper&) = default;
		ModelWrapper(ModelWrapper&&) noexcept = default;
		ModelWrapper& operator=(const ModelWrapper&) = default;
		ModelWrapper& operator=(ModelWrapper&&) noexcept = default;
		virtual ~ModelWrapper() { this->_ref.reset(); }

		// UTILS ----
		[[nodiscard]] virtual const rawrbox::Vector3f getPos() const {
			if (!this->isValid()) return {};
			return this->_ref.lock()->getPos();
		}

		virtual void setPos(const rawrbox::Vector3f& pos) {
			if (!this->isValid()) return;
			this->_ref.lock()->setPos(pos);
		}

		[[nodiscard]] virtual const rawrbox::Vector3f getScale() const {
			if (!this->isValid()) return {};
			return this->_ref.lock()->getScale();
		}

		virtual void setScale(const rawrbox::Vector3f& scale) {
			if (!this->isValid()) return;
			this->_ref.lock()->setScale(scale);
		}

		[[nodiscard]] virtual const rawrbox::Vector4f getAngle() const {
			if (!this->isValid()) return {};
			return this->_ref.lock()->getAngle();
		}

		virtual void setAngle(const rawrbox::Vector4f& ang) {
			if (!this->isValid()) return;
			this->_ref.lock()->setAngle(ang);
		}

		virtual void setEulerAngle(const rawrbox::Vector3f& ang) {
			if (!this->isValid()) return;
			this->_ref.lock()->setEulerAngle(ang);
		}

		[[nodiscard]] virtual const rawrbox::Matrix4x4 getMatrix() const {
			if (!this->isValid()) return {};
			return this->_ref.lock()->getMatrix();
		}

		[[nodiscard]] virtual bool isDynamic() const {
			if (!this->isValid()) return false;
			return this->_ref.lock()->isDynamic();
		}

		[[nodiscard]] virtual bool isUploaded() const {
			if (!this->isValid()) return false;
			return this->_ref.lock()->isUploaded();
		}
		// ------

		[[nodiscard]] bool isValid() const {
			return !this->_ref.expired();
		}

		static void registerLua(sol::state& lua) {
			lua.new_usertype<ModelWrapper<M>>("ModelBase<>",
			    sol::no_constructor,

			    // UTILS ----
			    "getPos", &ModelWrapper<M>::getPos,
			    "setPos", &ModelWrapper<M>::setPos,

			    "getScale", &ModelWrapper<M>::getScale,
			    "setScale", &ModelWrapper<M>::setScale,

			    "getAngle", &ModelWrapper<M>::getAngle,
			    "setAngle", &ModelWrapper<M>::setAngle,
			    "setEulerAngle", &ModelWrapper<M>::setEulerAngle,

			    "getMatrix", &ModelWrapper<M>::getMatrix,
			    "isDynamic", &ModelWrapper<M>::isDynamic,
			    "isUploaded", &ModelWrapper<M>::isUploaded,
			    // --------------

			    "isValid", &ModelWrapper<M>::isValid);
		}
	};
} // namespace rawrbox
