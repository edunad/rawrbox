
#include <rawrbox/render/camera/base.hpp>

#include <bx/math.h>

namespace rawrbox {
	// UTILS -----
	void CameraBase::setPos(const rawrbox::Vector3f& pos) {
		this->_pos = pos;
		this->updateMtx();
	}

	const rawrbox::Vector3f& CameraBase::getPos() {
		return this->_pos;
	}

	void CameraBase::setAngle(const rawrbox::Quaternion& angle) {
		this->_angle = angle;
		this->updateMtx();
	}

	const rawrbox::Quaternion& CameraBase::getAngle() {
		return this->_angle;
	}

	rawrbox::Vector3f CameraBase::getForward() {
		return {
		    bx::cos(this->_angle.y) * bx::sin(this->_angle.x),
		    bx::sin(this->_angle.y),
		    bx::cos(this->_angle.y) * bx::cos(this->_angle.x),
		};
	}

	rawrbox::Vector3f CameraBase::getRight() {
		return {
		    bx::sin(this->_angle.x - bx::kPiHalf),
		    0.0F,
		    bx::cos(this->_angle.x - bx::kPiHalf),
		};
	}

	rawrbox::Vector3f CameraBase::getUp() {
		auto right = this->getRight();
		auto forward = this->getForward();

		auto up = bx::cross({right.x, right.y, right.z}, {forward.x, forward.y, forward.z});
		return {up.x, up.y, up.z};
	}

	std::array<float, 16>& CameraBase::getViewMtx() {
		return this->_view;
	}

	std::array<float, 16>& CameraBase::getProjMtx() {
		return this->_projection;
	}

	// ----------------
	void CameraBase::updateMtx() {}
} // namespace rawrbox
