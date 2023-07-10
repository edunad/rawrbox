#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/camera/base.hpp>
#include <rawrbox/render/window.hpp>

namespace rawrbox {
	class CameraPerspective : public rawrbox::CameraBase {
	protected:
		rawrbox::Vector2i _winSize = {};
		float _FOV = 60.F;

		void updateMtx() override;

	public:
		CameraPerspective(const CameraPerspective&) = default;
		CameraPerspective(CameraPerspective&&) = default;
		CameraPerspective& operator=(const CameraPerspective&) = default;
		CameraPerspective& operator=(CameraPerspective&&) = default;
		~CameraPerspective() override = default;

		explicit CameraPerspective(const rawrbox::Vector2i& winSize, float FOV = 60.F, float near = 0.1F, float far = 45.F);

		[[nodiscard]] const rawrbox::Vector3f worldToScreen(const rawrbox::Vector3f& pos) const override;
		[[nodiscard]] const rawrbox::Vector3f screenToWorld(const rawrbox::Vector2f& screen_pos) const override;
	};
} // namespace rawrbox
