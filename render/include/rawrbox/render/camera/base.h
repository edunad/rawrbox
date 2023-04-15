#pragma once
#include <rawrbox/math/quaternion.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>

#include <array>

namespace rawrBox {
	class CameraBase {
	protected:
		rawrBox::Vector3f _pos;
		rawrBox::Quaternion _angle;

		std::array<float, 16> _view;
		std::array<float, 16> _projection;

		virtual void update();

	public:
		CameraBase();
		virtual ~CameraBase() = default;

		// UTILS -----
		virtual void setPos(const rawrBox::Vector3f& pos);
		virtual const rawrBox::Vector3f& getPos();

		virtual void setAngle(const rawrBox::Quaternion& ang);
		virtual const rawrBox::Quaternion& getAngle();

		virtual rawrBox::Vector3f getForward();
		virtual rawrBox::Vector3f getRight();
		virtual rawrBox::Vector3f getUp();

		virtual std::array<float, 16>& getViewMtx();
		virtual std::array<float, 16>& getProjMtx();
		// ----------------
	};
} // namespace rawrBox
