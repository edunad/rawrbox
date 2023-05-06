#pragma once
#include <rawrbox/math/quaternion.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>

#include <array>

namespace rawrbox {
	class CameraBase {
	protected:
		rawrbox::Vector3f _pos;
		rawrbox::Quaternion _angle;

		std::array<float, 16> _view = {};
		std::array<float, 16> _projection = {};

		virtual void updateMtx();

	public:
		virtual ~CameraBase() = default;

		CameraBase() = default;
		CameraBase(CameraBase&&) = delete;
		CameraBase& operator=(CameraBase&&) = delete;
		CameraBase(const CameraBase&) = delete;
		CameraBase& operator=(const CameraBase&) = delete;

		// UTILS -----
		virtual void setPos(const rawrbox::Vector3f& pos);
		virtual const rawrbox::Vector3f& getPos();

		virtual void setAngle(const rawrbox::Quaternion& ang);
		virtual const rawrbox::Quaternion& getAngle();

		virtual rawrbox::Vector3f getForward();
		virtual rawrbox::Vector3f getRight();
		virtual rawrbox::Vector3f getUp();

		virtual std::array<float, 16>& getViewMtx();
		virtual std::array<float, 16>& getProjMtx();
		// ----------------
	};
} // namespace rawrbox
