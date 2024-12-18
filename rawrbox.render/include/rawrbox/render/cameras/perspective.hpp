#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/window.hpp>

namespace rawrbox {
	class CameraPerspective : public rawrbox::CameraBase {
	protected:
		float _FOV = 60.F;
		void updateMtx() override;

	public:
		CameraPerspective(const CameraPerspective&) = delete;
		CameraPerspective(CameraPerspective&&) = default;
		CameraPerspective& operator=(const CameraPerspective&) = delete;
		CameraPerspective& operator=(CameraPerspective&&) = default;
		~CameraPerspective() override = default;

		explicit CameraPerspective(const rawrbox::Vector2u& renderSize, float FOV = 60.F, float near = 0.01F, float far = 100.F, bool depth = true);

		[[nodiscard]] rawrbox::Vector3f worldToScreen(const rawrbox::Vector3f& pos) const override;
		[[nodiscard]] rawrbox::Vector3f screenToWorld(const rawrbox::Vector2f& screenPos, const rawrbox::Vector3f& origin = {0, 0, 0}) const override;
	};
} // namespace rawrbox
