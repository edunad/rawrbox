
#include <rawrbox/render/camera/perspective.hpp>

#include <bx/math.h>

namespace rawrbox {
	// NOLINTBEGIN(clang-analyzer-optin.cplusplus.VirtualCall)
	CameraPerspective::CameraPerspective(float ratio, float FOV, float near, float far, bool homogeneousDepth) {
		bx::mtxProj(this->_projection.data(), FOV, ratio, near, far, homogeneousDepth);
		this->update();
	}
	// NOLINTEND(clang-analyzer-optin.cplusplus.VirtualCall)

	void CameraPerspective::update() {
		auto dir = this->getForward();

		auto m_eye = bx::Vec3(this->_pos.x, this->_pos.y, this->_pos.z);
		auto m_at = bx::add(m_eye, {dir.x, dir.y, dir.z});
		auto m_up = this->getUp();

		bx::mtxLookAt(this->_view.data(), m_eye, m_at, {m_up.x, m_up.y, m_up.z});
	}
} // namespace rawrbox
