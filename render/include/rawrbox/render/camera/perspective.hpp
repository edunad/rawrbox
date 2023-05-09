#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/camera/base.hpp>
#include <rawrbox/render/window.hpp>

namespace rawrbox {
	class CameraPerspective : public CameraBase {
	protected:
		rawrbox::Window* _window = nullptr;

		float _FOV = 60.F;
		float _near = 0.1F;
		float _far = 100.F;
		bool _homogeneousDepth = false;

		void updateMtx() override;

	public:
		explicit CameraPerspective(rawrbox::Window* window, float FOV = 60.F, float near = 0.1F, float far = 100.F, bool homogeneousDepth = false);

		[[nodiscard]] const rawrbox::Vector3i worldToScreen(const rawrbox::Vector3& pos) const override;
		[[nodiscard]] const rawrbox::Vector3 screenToWorld(const rawrbox::Vector2i& screen_pos) const override;
	};
} // namespace rawrbox
