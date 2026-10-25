#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/cameras/base.hpp>

namespace rawrbox {
	class CameraOrtho : public rawrbox::CameraBase {
	protected:
		rawrbox::Vector2f _extent = {1.F, 1.F};
		float _eyeDistance = 100.F;

		void updateMtx() override;

	public:
		explicit CameraOrtho(const rawrbox::Vector2u& renderSize, float eyeDistance = 100.F, bool depth = true);
		CameraOrtho(const CameraOrtho&) = delete;
		CameraOrtho(CameraOrtho&&) = delete;
		CameraOrtho& operator=(const CameraOrtho&) = delete;
		CameraOrtho& operator=(CameraOrtho&&) = delete;
		~CameraOrtho() override = default;

		virtual void setExtent(const rawrbox::Vector2f& extent);
		[[nodiscard]] virtual const rawrbox::Vector2f& getExtent() const;

		[[nodiscard]] rawrbox::Vector3f worldToScreen(const rawrbox::Vector3f& pos) const override;
		[[nodiscard]] rawrbox::Vector3f screenToWorld(const rawrbox::Vector2f& screenPos, const rawrbox::Vector3f& origin = {0, 0, 0}) const override;
		[[nodiscard]] rawrbox::Vector3f screenRayDir(const rawrbox::Vector2f& screenPos) const override;
	};
} // namespace rawrbox