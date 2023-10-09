#pragma once

#include <rawrbox/render_temp/camera/base.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class CameraWrapper {
	public:
		CameraWrapper() = default;
		CameraWrapper(const CameraWrapper&) = default;
		CameraWrapper(CameraWrapper&&) = default;
		CameraWrapper& operator=(const CameraWrapper&) = default;
		CameraWrapper& operator=(CameraWrapper&&) = default;
		virtual ~CameraWrapper() = default;

		// UTILS -----
		virtual void setPos(const rawrbox::Vector3f& pos);
		[[nodiscard]] virtual const rawrbox::Vector3f getPos() const;

		virtual void setAngle(const rawrbox::Vector4f& ang);
		[[nodiscard]] virtual const rawrbox::Vector4f getAngle() const;

		[[nodiscard]] virtual rawrbox::Vector3f getForward() const;
		[[nodiscard]] virtual rawrbox::Vector3f getRight() const;
		[[nodiscard]] virtual rawrbox::Vector3f getUp() const;

		[[nodiscard]] virtual float getZFar() const;
		[[nodiscard]] virtual float getZNear() const;

		[[nodiscard]] virtual const rawrbox::Matrix4x4 getViewMtx() const;
		[[nodiscard]] virtual const rawrbox::Matrix4x4 getProjMtx() const;
		[[nodiscard]] virtual const rawrbox::Matrix4x4 getProjViewMtx() const;

		[[nodiscard]] virtual const rawrbox::Vector3f worldToScreen(const rawrbox::Vector3f& pos) const;
		[[nodiscard]] virtual const rawrbox::Vector3f screenToWorld(const rawrbox::Vector2f& screen_pos) const;
		// ----------------

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
