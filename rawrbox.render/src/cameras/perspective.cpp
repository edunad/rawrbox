

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
		auto dir = this->getForward();

		auto at = this->_pos + dir;
		auto up = this->getUp();

		this->_view = rawrbox::Matrix4x4::mtxLookAt(this->_pos, at, up);
	}

	rawrbox::Vector3f CameraPerspective::worldToScreen(const rawrbox::Vector3f& pos) const {
		const auto& size = this->_renderTarget->getSize();
		return rawrbox::Matrix4x4::mtxProject(pos, this->_view, this->_projection, {0, 0, size.x, size.y});
	}

	rawrbox::Vector3f CameraPerspective::screenToWorld(const rawrbox::Vector2f& screenPos, const rawrbox::Vector3f& origin) const {
		static constexpr rawrbox::Vector3f plane_normal = {0, 1, 0};

		// get our pos and force aim downwards, the getForward() seems to behave odd when aiming full down
		const auto& campos = this->getPos();
		const auto& size = this->_renderTarget->getSize();
		rawrbox::Matrix4x4 viewproj_inv = this->getViewProjMtx().inverse();

		auto winPos = size.cast<float>();
		float screenx_clip = 2.F * (screenPos.x / winPos.x) - 1.F;
		float screeny_clip = 1.F - 2.F * (screenPos.y) / winPos.y;

		rawrbox::Vector4f screen_clip = {screenx_clip, screeny_clip, -1, 1};
		rawrbox::Vector4f world_pos = viewproj_inv.mulVec(screen_clip);

		// divide by the weigth of the universe to resolve black mater offsets
		world_pos /= world_pos.w;

		// convert the object back to the real universe
		rawrbox::Vector3f mouse_point_world = {world_pos.x, world_pos.y, world_pos.z};
		rawrbox::Vector3f camera_forward_world = mouse_point_world - campos;

		float numerator = (origin - campos).dot(plane_normal);
		float denumerator = camera_forward_world.dot(plane_normal);

		float delta = numerator / denumerator;
		return camera_forward_world * delta + campos;
	}

} // namespace rawrbox
