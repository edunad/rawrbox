

#include <rawrbox/render/cameras/perspective.hpp>

namespace rawrbox {
	// NOLINTBEGIN(clang-analyzer-optin.cplusplus.VirtualCall)
	CameraPerspective::CameraPerspective(const rawrbox::Vector2u& renderSize, float FOV, float near, float far, bool depth) : rawrbox::CameraBase(renderSize, depth), _FOV(FOV) {
		this->_z_near = near;
		this->_z_far = far;

		this->_projection = rawrbox::Matrix4x4::mtxProj(FOV, static_cast<float>(renderSize.x) / static_cast<float>(renderSize.y), this->_z_near, this->_z_far);
		this->updateMtx();
	}
	// NOLINTEND(clang-analyzer-optin.cplusplus.VirtualCall)

	void CameraPerspective::updateMtx() {
		const auto dir = this->getForward();
		const auto eye = this->getEyePos();

		const auto at = eye + dir;
		const auto up = this->getUp();

		this->_view = rawrbox::Matrix4x4::mtxLookAt(eye, at, up);
	}

	rawrbox::Vector3f CameraPerspective::worldToScreen(const rawrbox::Vector3f& pos) const {
		const auto& size = this->_renderTarget->getSize();
		return rawrbox::Matrix4x4::mtxProject(pos, this->_view, this->_projection, {0, 0, size.x, size.y});
	}

	rawrbox::Vector3f CameraPerspective::screenRayDir(const rawrbox::Vector2f& screenPos) const {
		const auto campos = this->getEyePos();

		const auto& size = this->_renderTarget->getSize();
		const rawrbox::Matrix4x4 viewproj_inv = this->getViewProjMtx().inverse();

		const auto winPos = size.cast<float>();
		const float screenx_clip = 2.F * (screenPos.x / winPos.x) - 1.F;
		const float screeny_clip = 1.F - 2.F * (screenPos.y) / winPos.y;

		const rawrbox::Vector4f screen_clip = {screenx_clip, screeny_clip, -1, 1};
		rawrbox::Vector4f world_pos = viewproj_inv.mulVec(screen_clip);
		world_pos /= world_pos.w;

		const rawrbox::Vector3f point = {world_pos.x, world_pos.y, world_pos.z};
		return (point - campos).normalized();
	}

	rawrbox::Vector3f CameraPerspective::screenToWorld(const rawrbox::Vector2f& screenPos, const rawrbox::Vector3f& origin) const {
		static constexpr rawrbox::Vector3f plane_normal = {0, 1, 0};

		const auto campos = this->getEyePos();
		const auto dir = this->screenRayDir(screenPos);

		const float numerator = (origin - campos).dot(plane_normal);
		const float denumerator = dir.dot(plane_normal);

		const float delta = numerator / denumerator;
		return dir * delta + campos;
	}

} // namespace rawrbox