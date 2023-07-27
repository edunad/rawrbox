
#include <rawrbox/render/camera/base.hpp>

#include <bx/math.h>

#include <stdexcept>

namespace rawrbox {
	void CameraBase::updateMtx(){};

	// UTILS -----
	void CameraBase::setPos(const rawrbox::Vector3f& pos) {
		this->_pos = pos;
		this->updateMtx();
	}

	const rawrbox::Vector3f& CameraBase::getPos() const {
		return this->_pos;
	}

	void CameraBase::setAngle(const rawrbox::Vector4f& angle) {
		this->_angle = angle;
		this->updateMtx();
	}

	const rawrbox::Vector4f& CameraBase::getAngle() const {
		return this->_angle;
	}

	float CameraBase::getZFar() const { return this->_z_far; }
	float CameraBase::getZNear() const { return this->_z_near; }

	rawrbox::Vector3f CameraBase::getForward() const {
		return {
		    bx::cos(this->_angle.y) * bx::sin(this->_angle.x),
		    bx::sin(this->_angle.y),
		    bx::cos(this->_angle.y) * bx::cos(this->_angle.x),
		};
	}

	rawrbox::Vector3f CameraBase::getRight() const {
		return {
		    bx::sin(this->_angle.x - bx::kPiHalf),
		    0.0F,
		    bx::cos(this->_angle.x - bx::kPiHalf),
		};
	}

	rawrbox::Vector3f CameraBase::getUp() const {
		auto right = this->getRight();
		auto forward = this->getForward();

		auto up = bx::cross({right.x, right.y, right.z}, {forward.x, forward.y, forward.z});
		return {up.x, up.y, up.z};
	}

	const rawrbox::Matrix4x4& CameraBase::getViewMtx() const {
		return this->_view;
	}

	const rawrbox::Matrix4x4& CameraBase::getProjMtx() const {
		return this->_projection;
	}

	const rawrbox::Matrix4x4 CameraBase::getProjViewMtx() const {
		return this->_projection * this->_view;
	}

	void CameraBase::update() {
		throw std::runtime_error("Not implemented");
	}

	const rawrbox::Vector3f CameraBase::worldToScreen(const rawrbox::Vector3f& /*pos*/) const {
		throw std::runtime_error("Not implemented");
	}

	const rawrbox::Vector3f CameraBase::screenToWorld(const rawrbox::Vector2f& /*screen_pos*/) const {
		throw std::runtime_error("Not implemented");
	}

	// ----------------
} // namespace rawrbox
