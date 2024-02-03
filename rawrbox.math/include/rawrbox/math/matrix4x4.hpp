#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <array>
#include <cstring>

namespace rawrbox {

	class Matrix4x4 {
	protected:
		void vec4MulMtx(float* _result, const float* _vec, const float* _mat);

		void mul(const rawrbox::Matrix4x4& other);
		void mul(const rawrbox::Vector3f& other);

		void add(const rawrbox::Matrix4x4& other);
		void add(const rawrbox::Vector3f& other);

	public:
		static bool MTX_RIGHT_HANDED;

		std::array<float, 16> mtx = {
		    1, 0, 0, 0,
		    0, 1, 0, 0,
		    0, 0, 1, 0,
		    0, 0, 0, 1}; // Identity

		Matrix4x4() = default;
		explicit Matrix4x4(const std::array<float, 16>& other);
		explicit Matrix4x4(const float* other);

		[[nodiscard]] size_t size() const;
		[[nodiscard]] const float* data() const;
		[[nodiscard]] float* data();

		// UTILS ----
		rawrbox::Matrix4x4& zero();
		rawrbox::Matrix4x4& identity();

		rawrbox::Matrix4x4& transpose();
		rawrbox::Matrix4x4& transpose(const float* other);
		rawrbox::Matrix4x4& transpose(const rawrbox::Matrix4x4& other);

		rawrbox::Matrix4x4& translate(const rawrbox::Vector3f& pos);
		rawrbox::Matrix4x4& scale(const rawrbox::Vector3f& scale);

		rawrbox::Matrix4x4& rotate(const rawrbox::Vector4f& rot);
		rawrbox::Matrix4x4& rotateX(float x);
		rawrbox::Matrix4x4& rotateY(float y);
		rawrbox::Matrix4x4& rotateZ(float z);
		rawrbox::Matrix4x4& rotateXYZ(const rawrbox::Vector3f& rot);

		// Views
		rawrbox::Matrix4x4& ortho(float left, float right, float bottom, float top, float near, float far);
		rawrbox::Matrix4x4& proj(float FOV, float aspect, float near, float far);
		rawrbox::Matrix4x4& billboard(const rawrbox::Matrix4x4& view, const rawrbox::Vector3f& pos, const rawrbox::Vector3f& scale);
		// ----------

		// Scale Rotate Translate
		rawrbox::Matrix4x4& SRT(const rawrbox::Vector3f& scale, const rawrbox::Vector4f& rotation, const rawrbox::Vector3f& pos);

		[[nodiscard]] rawrbox::Vector3f mulVec(const rawrbox::Vector3f& other) const;
		[[nodiscard]] rawrbox::Vector4f mulVec(const rawrbox::Vector4f& other) const;

		rawrbox::Matrix4x4& inverse();
		rawrbox::Matrix4x4& lookAt(const rawrbox::Vector3f& _eye, const rawrbox::Vector3f& _at, const rawrbox::Vector3f& _up);
		// ------

		// STATIC UTILS ----
		static rawrbox::Matrix4x4 mtxTranspose(rawrbox::Matrix4x4 mtx);
		static rawrbox::Matrix4x4 mtxInverse(rawrbox::Matrix4x4 mtx);

		static rawrbox::Matrix4x4 mtxSRT(const rawrbox::Vector3f& scale, const rawrbox::Vector4f& rotation, const rawrbox::Vector3f& pos);

		static rawrbox::Matrix4x4 mtxLookAt(const rawrbox::Vector3f& eye, const rawrbox::Vector3f& at, const rawrbox::Vector3f& up);
		static rawrbox::Matrix4x4 mtxOrtho(float left, float right, float bottom, float top, float near, float far);
		static rawrbox::Matrix4x4 mtxProj(float FOV, float aspect, float near, float far);

		static rawrbox::Vector3f mtxProject(const rawrbox::Vector3f& pos, const rawrbox::Matrix4x4& view, const rawrbox::Matrix4x4& proj, const rawrbox::Vector4i& viewport);
		// ------

		// OPERATORS ----
		float operator[](const size_t indx) const;
		float operator[](const size_t indx);

		void operator/=(const float other);
		void operator/=(rawrbox::Matrix4x4 other);

		void operator*=(const rawrbox::Matrix4x4& other);
		void operator*=(const rawrbox::Vector3f& other);

		rawrbox::Matrix4x4 operator*(rawrbox::Matrix4x4 other) const;
		rawrbox::Matrix4x4 operator*(rawrbox::Vector3f other) const;

		rawrbox::Matrix4x4 operator+(rawrbox::Matrix4x4 other) const;
		rawrbox::Matrix4x4 operator+(rawrbox::Vector3f other) const;

		rawrbox::Matrix4x4 operator/(rawrbox::Matrix4x4 other) const;

		bool operator==(const rawrbox::Matrix4x4& other) const;
		bool operator!=(const rawrbox::Matrix4x4& other) const;
		// ------
	};
} // namespace rawrbox
