#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/materials/base.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	template <typename T>
	class ModelBase;

	class ModelBaseWrapper {
	protected:
		std::weak_ptr<rawrbox::ModelBase<rawrbox::MaterialBase>> _ref;

	public:
		ModelBaseWrapper(const std::shared_ptr<rawrbox::ModelBase<rawrbox::MaterialBase>>& ref);
		ModelBaseWrapper(const ModelBaseWrapper&) = default;
		ModelBaseWrapper(ModelBaseWrapper&&) = default;
		ModelBaseWrapper& operator=(const ModelBaseWrapper&) = default;
		ModelBaseWrapper& operator=(ModelBaseWrapper&&) = default;
		virtual ~ModelBaseWrapper();

		// UTILS ----
		[[nodiscard]] virtual const rawrbox::Vector3f getPos() const;
		virtual void setPos(const rawrbox::Vector3f& pos);

		[[nodiscard]] virtual const rawrbox::Vector3f getScale() const;
		virtual void setScale(const rawrbox::Vector3f& scale);

		[[nodiscard]] virtual const rawrbox::Vector4f getAngle() const;
		virtual void setAngle(const rawrbox::Vector4f& ang);

		virtual void setEulerAngle(const rawrbox::Vector3f& ang);

		[[nodiscard]] virtual const rawrbox::Matrix4x4 getMatrix() const;

		[[nodiscard]] virtual bool isDynamic() const;

		[[nodiscard]] virtual bool isUploaded() const;
		// ------

		[[nodiscard]] bool isValid() const;

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
