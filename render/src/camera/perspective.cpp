

#include <rawrbox/render/camera/perspective.hpp>

#include <bx/math.h>

#include <stdexcept>

namespace rawrbox {
	// NOLINTBEGIN(clang-analyzer-optin.cplusplus.VirtualCall)
	CameraPerspective::CameraPerspective(rawrbox::Window* window, float FOV, float near, float far, bool homogeneousDepth) : _window(window), _FOV(FOV), _near(near), _far(far), _homogeneousDepth(homogeneousDepth) {
		bx::mtxProj(this->_projection.data(), FOV, window->getAspectRatio(), near, far, homogeneousDepth);
		this->updateMtx();
	}
	// NOLINTEND(clang-analyzer-optin.cplusplus.VirtualCall)

	void CameraPerspective::updateMtx() {
		auto dir = this->getForward();

		auto m_at = this->_pos + dir;
		auto m_up = this->getUp();

		this->_view.lookAt(this->_pos, m_at, m_up);
	}

	const rawrbox::Vector3i CameraPerspective::worldToScreen(const rawrbox::Vector3& pos) const {
		/*auto out = rawrbox::Matrix4x4::project(glm::vec3(pos.x, pos.y, pos.z), viewMat, projMat, {0, _winSize.y, _winSize.x, -_winSize.y});

		return {out.x, out.y, out.z};*/
		throw std::runtime_error("a");
	}

	const rawrbox::Vector3 CameraPerspective::screenToWorld(const rawrbox::Vector2i& screen_pos) const {
		/*
		math::Vector3 plane_origin {0, 0, 0};
		math::Vector3 plane_normal {0, 0, 1};
		auto windowSizeFloat = windowSize.cast<float>();
		auto screenPosFloat = screen_pos.cast<float>();

		// get our pos and force aim downwards, the getForward() seems to behave odd when aiming full down
		auto campos = getLocation();

		// <insert zooperdiebap math animation>
		glm::mat4x4 viewproj_inv = glm::inverse(projMat * viewMat);

		float screenx_clip = 2 * (screenPosFloat.x / windowSizeFloat.x) - 1;
		float screeny_clip = 1 - 2 * (screenPosFloat.y) / windowSizeFloat.y;

		glm::vec4 screen_clip = {screenx_clip, screeny_clip, -1, 1};
		glm::vec4 world_pos = viewproj_inv * screen_clip;

		// divide by the weigth of the universe to resolve black mater offsets
		world_pos /= world_pos.w;

		// convert the object back to the real universe
		math::Vector3 mouse_point_world = {world_pos.x, world_pos.y, world_pos.z};
		math::Vector3 camera_forward_world = mouse_point_world - campos;

		float numerator = (plane_origin - campos).dot(plane_normal);
		float denumerator = camera_forward_world.dot(plane_normal);

		float delta = numerator / denumerator;
		math::Vector3 aimPos = camera_forward_world * delta + campos;

		return aimPos;*/
		throw std::runtime_error("a");
	}

} // namespace rawrbox
