#pragma once
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <array>

namespace rawrbox {
	class CameraBase {
	protected:
		rawrbox::Vector3f _pos = {};
		rawrbox::Vector4f _angle = {};

		rawrbox::Matrix4x4 _view = {};
		rawrbox::Matrix4x4 _projection = {};

		virtual void updateMtx();

	public:
		virtual ~CameraBase() = default;

		CameraBase() = default;
		CameraBase(CameraBase&&) = delete;
		CameraBase& operator=(CameraBase&&) = delete;
		CameraBase(const CameraBase&) = delete;
		CameraBase& operator=(const CameraBase&) = delete;

		// UTILS -----
		virtual void setPos(const rawrbox::Vector3f& pos);
		[[nodiscard]] virtual const rawrbox::Vector3f& getPos() const;

		virtual void setAngle(const rawrbox::Vector4f& ang);
		[[nodiscard]] virtual const rawrbox::Vector4f& getAngle() const;

		[[nodiscard]] virtual rawrbox::Vector3f getForward() const;
		[[nodiscard]] virtual rawrbox::Vector3f getRight() const;
		[[nodiscard]] virtual rawrbox::Vector3f getUp() const;

		[[nodiscard]] virtual const rawrbox::Matrix4x4& getViewMtx() const;
		[[nodiscard]] virtual const rawrbox::Matrix4x4& getProjMtx() const;

		[[nodiscard]] virtual const rawrbox::Vector3f worldToScreen(const rawrbox::Vector3f& pos) const;
		[[nodiscard]] virtual const rawrbox::Vector3f screenToWorld(const rawrbox::Vector2f& screen_pos) const;
		// ----------------

		virtual void update();
	};
} // namespace rawrbox
