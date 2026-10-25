#include <rawrbox/render/cameras/ortho.hpp>

namespace rawrbox {
	// NOLINTBEGIN(clang-analyzer-optin.cplusplus.VirtualCall)
	CameraOrtho::CameraOrtho(const rawrbox::Vector2u& renderSize, float eyeDistance, bool depth) : rawrbox::CameraBase(renderSize, depth), _eyeDistance(eyeDistance) {
		this->_z_near = 0.F;
		this->_z_far = eyeDistance * 2.F;

		this->setExtent(renderSize.cast<float>());
		this->updateMtx();
	}
	// NOLINTEND(clang-analyzer-optin.cplusplus.VirtualCall)

	void CameraOrtho::updateMtx() {
		const auto forward = this->getForward();
		const auto eye = this->_pos - forward * this->_eyeDistance;

		this->_view = rawrbox::Matrix4x4::mtxLookAt(eye, eye + forward, this->getUp());
	}

	void CameraOrtho::setExtent(const rawrbox::Vector2f& extent) {
		this->_extent = extent;
		this->_projection = rawrbox::Matrix4x4::mtxOrtho(-extent.x * 0.5F, extent.x * 0.5F, -extent.y * 0.5F, extent.y * 0.5F, this->_z_near, this->_z_far);
	}

	const rawrbox::Vector2f& CameraOrtho::getExtent() const { return this->_extent; }

	rawrbox::Vector3f CameraOrtho::worldToScreen(const rawrbox::Vector3f& pos) const {
		const auto clip = this->getViewProjMtx().mulVec(rawrbox::Vector4f{pos.x, pos.y, pos.z, 1.F});
		const auto size = this->_renderTarget->getSize().cast<float>();

		return {((clip.x * 0.5F) + 0.5F) * size.x, (0.5F - (clip.y * 0.5F)) * size.y, clip.z};
	}

	rawrbox::Vector3f CameraOrtho::screenToWorld(const rawrbox::Vector2f& screenPos, const rawrbox::Vector3f& origin) const {
		const auto size = this->_renderTarget->getSize().cast<float>();
		const auto near = this->getViewProjMtx().inverse().mulVec(rawrbox::Vector4f{((screenPos.x / size.x) * 2.F) - 1.F, 1.F - ((screenPos.y / size.y) * 2.F), 0.F, 1.F});

		const rawrbox::Vector3f point = {near.x, near.y, near.z};
		const auto forward = this->getForward();

		return point + forward * ((origin.y - point.y) / forward.y);
	}

	rawrbox::Vector3f CameraOrtho::screenRayDir(const rawrbox::Vector2f& /*screenPos*/) const {
		return this->getForward();
	}
} // namespace rawrbox