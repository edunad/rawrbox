#pragma once

#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class CameraGlobal {
	public:
		// UTILS -----
		static void setPos(const rawrbox::Vector3f& pos);
		[[nodiscard]] static rawrbox::Vector3f getPos();

		static void setAngle(const rawrbox::Vector4f& ang);
		[[nodiscard]] static rawrbox::Vector4f getAngle();

		[[nodiscard]] static rawrbox::Vector3f getForward();
		[[nodiscard]] static rawrbox::Vector3f getRight();
		[[nodiscard]] static rawrbox::Vector3f getUp();

		[[nodiscard]] static float getZFar();
		[[nodiscard]] static float getZNear();

		[[nodiscard]] static rawrbox::Matrix4x4 getViewMtx();
		[[nodiscard]] static rawrbox::Matrix4x4 getProjMtx();
		[[nodiscard]] static rawrbox::Matrix4x4 getViewProjMtx();

		[[nodiscard]] static rawrbox::Vector3f worldToScreen(const rawrbox::Vector3f& pos);
		[[nodiscard]] static rawrbox::Vector3f screenToWorld(const rawrbox::Vector2f& screen_pos);
		// ----------------

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
