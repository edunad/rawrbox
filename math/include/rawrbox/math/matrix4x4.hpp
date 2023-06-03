#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <bx/math.h>

#include <algorithm>
#include <array>
#include <cstring>

namespace rawrbox {

	class Matrix4x4 {
	protected:
		void vec4MulMtx(float* _result, const float* _vec, const float* _mat) {
			_result[0] = _vec[0] * _mat[0] + _vec[1] * _mat[4] + _vec[2] * _mat[8] + _vec[3] * _mat[12];
			_result[1] = _vec[0] * _mat[1] + _vec[1] * _mat[5] + _vec[2] * _mat[9] + _vec[3] * _mat[13];
			_result[2] = _vec[0] * _mat[2] + _vec[1] * _mat[6] + _vec[2] * _mat[10] + _vec[3] * _mat[14];
			_result[3] = _vec[0] * _mat[3] + _vec[1] * _mat[7] + _vec[2] * _mat[11] + _vec[3] * _mat[15];
		}

	public:
		std::array<float, 16> mtx = {
		    1, 0, 0, 0,
		    0, 1, 0, 0,
		    0, 0, 1, 0,
		    0, 0, 0, 1}; // Identity

		Matrix4x4() = default;
		explicit Matrix4x4(const std::array<float, 16>& other) { std::memcpy(this->mtx.data(), other.data(), sizeof(float) * this->mtx.size()); };
		explicit Matrix4x4(const float* other) { std::memcpy(this->mtx.data(), other, sizeof(float) * this->mtx.size()); };

		[[nodiscard]] size_t size() const { return this->mtx.size(); }
		[[nodiscard]] const float* data() const { return this->mtx.data(); }
		[[nodiscard]] float* data() { return this->mtx.data(); }

		// UTILS ----
		void zero() {
			std::memset(this->mtx.data(), 0, sizeof(float) * this->mtx.size());
		}

		void identity() {
			this->mtx = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
		}

		void transpose(const float* other) {
			this->mtx[0] = other[0];
			this->mtx[4] = other[1];
			this->mtx[8] = other[2];
			this->mtx[12] = other[3];
			this->mtx[1] = other[4];
			this->mtx[5] = other[5];
			this->mtx[9] = other[6];
			this->mtx[13] = other[7];
			this->mtx[2] = other[8];
			this->mtx[6] = other[9];
			this->mtx[10] = other[10];
			this->mtx[14] = other[11];
			this->mtx[3] = other[12];
			this->mtx[7] = other[13];
			this->mtx[11] = other[14];
			this->mtx[15] = other[15];
		}

		void transpose(const std::array<float, 16>& other) {
			this->transpose(other.data());
		}

		void translate(const rawrbox::Vector3f& pos) {
			this->mtx[12] = pos.x;
			this->mtx[13] = pos.y;
			this->mtx[14] = pos.z;
		}

		void scale(const rawrbox::Vector3f& scale) {
			this->mtx[0] = scale.x;
			this->mtx[5] = scale.y;
			this->mtx[10] = scale.z;
		}

		void rotate(const rawrbox::Vector4f& rot) {
			const float x2 = rot.x + rot.x;
			const float y2 = rot.y + rot.y;
			const float z2 = rot.z + rot.z;
			const float x2x = x2 * rot.x;
			const float x2y = x2 * rot.y;
			const float x2z = x2 * rot.z;
			const float x2w = x2 * rot.w;
			const float y2y = y2 * rot.y;
			const float y2z = y2 * rot.z;
			const float y2w = y2 * rot.w;
			const float z2z = z2 * rot.z;
			const float z2w = z2 * rot.w;

			this->mtx[0] = 1.0F - (y2y + z2z);
			this->mtx[4] = x2y - z2w;
			this->mtx[8] = x2z + y2w;

			this->mtx[1] = x2y + z2w;
			this->mtx[5] = 1.0F - (x2x + z2z);
			this->mtx[9] = y2z - x2w;

			this->mtx[2] = x2z - y2w;
			this->mtx[6] = y2z + x2w;
			this->mtx[10] = 1.0F - (x2x + y2y);
		}

		void rotateX(float _ax) {
			const float sx = sin(_ax);
			const float cx = cos(_ax);

			this->mtx[5] = cx;
			this->mtx[6] = -sx;
			this->mtx[9] = sx;
			this->mtx[10] = cx;
		}

		void rotateY(float _ay) {
			const float sy = sin(_ay);
			const float cy = cos(_ay);

			this->mtx[0] = cy;
			this->mtx[2] = sy;
			this->mtx[8] = -sy;
			this->mtx[10] = cy;
		}

		void rotateZ(float _az) {
			const float sz = sin(_az);
			const float cz = cos(_az);

			this->mtx[0] = cz;
			this->mtx[1] = -sz;
			this->mtx[4] = sz;
			this->mtx[5] = cz;
		}

		void rotateXYZ(const rawrbox::Vector3f& rot) {
			Matrix4x4 x = {};
			x.rotateX(rot.x);

			Matrix4x4 y = {};
			y.rotateY(rot.y);

			Matrix4x4 z = {};
			z.rotateZ(rot.z);

			this->mtx = (x * y * z).mtx;
		}

		void mtxSRT(const rawrbox::Vector3f& scale, const rawrbox::Vector4f& angle, const rawrbox::Vector3f& pos) {
			rawrbox::Matrix4x4 mt = {};
			mt.translate(pos);

			rawrbox::Matrix4x4 ms = {};
			ms.scale(scale);

			rawrbox::Matrix4x4 mr = {};
			mr.rotate(angle); // Angle should be in world coords

			this->mtx = (mt * mr * ms).mtx;
		}

		void mtxSRT(const rawrbox::Vector3f& scale, const rawrbox::Vector3f& angle, const rawrbox::Vector3f& pos) {
			const float sx = sin(angle.x);
			const float cx = cos(angle.x);
			const float sy = sin(angle.y);
			const float cy = cos(angle.y);
			const float sz = sin(angle.z);
			const float cz = cos(angle.z);

			const float sxsz = sx * sz;
			const float cycz = cy * cz;

			this->mtx[0] = scale.x * (cycz - sxsz * sy);
			this->mtx[1] = scale.x * -cx * sz;
			this->mtx[2] = scale.x * (cz * sy + cy * sxsz);
			this->mtx[3] = 0.0F;

			this->mtx[4] = scale.y * (cz * sx * sy + cy * sz);
			this->mtx[5] = scale.y * cx * cz;
			this->mtx[6] = scale.y * (sy * sz - cycz * sx);
			this->mtx[7] = 0.0F;

			this->mtx[8] = scale.z * -cx * sy;
			this->mtx[9] = scale.z * sx;
			this->mtx[10] = scale.z * cx * cy;
			this->mtx[11] = 0.0F;

			this->mtx[12] = pos.x;
			this->mtx[13] = pos.y;
			this->mtx[14] = pos.z;
			this->mtx[15] = 1.0F;
		}

		void mul(const rawrbox::Matrix4x4& other) {
			rawrbox::Matrix4x4 _result;

			vec4MulMtx(&_result.mtx[0], &this->mtx[0], other.data());
			vec4MulMtx(&_result.mtx[4], &this->mtx[4], other.data());
			vec4MulMtx(&_result.mtx[8], &this->mtx[8], other.data());
			vec4MulMtx(&_result.mtx[12], &this->mtx[12], other.data());

			std::memcpy(this->mtx.data(), _result.data(), sizeof(float) * this->mtx.size());
		}

		void mul(const rawrbox::Vector3f& other) {
			rawrbox::Matrix4x4 _result;
			std::array<float, 4> m = {other.x, other.y, other.z, 1.F};

			vec4MulMtx(&_result.mtx[0], m.data(), this->data());
			vec4MulMtx(&_result.mtx[4], m.data(), this->data());
			vec4MulMtx(&_result.mtx[8], m.data(), this->data());
			vec4MulMtx(&_result.mtx[12], m.data(), this->data());

			std::memcpy(this->mtx.data(), _result.data(), sizeof(float) * this->mtx.size());
		}

		[[nodiscard]] rawrbox::Vector3f const mulVec(const rawrbox::Vector3f& other) const {
			rawrbox::Vector3f result = {};

			result.x = other.x * this->mtx[0] + other.y * this->mtx[4] + other.z * this->mtx[8] + this->mtx[12];
			result.y = other.x * this->mtx[1] + other.y * this->mtx[5] + other.z * this->mtx[9] + this->mtx[13];
			result.z = other.x * this->mtx[2] + other.y * this->mtx[6] + other.z * this->mtx[10] + this->mtx[14];

			return result;
		}

		[[nodiscard]] rawrbox::Vector4f const mulVec(const rawrbox::Vector4f& other) const {
			rawrbox::Vector4f result = {};

			result.x = other.x * this->mtx[0] + other.y * this->mtx[4] + other.z * this->mtx[8] + other.w * this->mtx[12];
			result.y = other.x * this->mtx[1] + other.y * this->mtx[5] + other.z * this->mtx[9] + other.w * this->mtx[13];
			result.z = other.x * this->mtx[2] + other.y * this->mtx[6] + other.z * this->mtx[10] + other.w * this->mtx[14];
			result.w = other.x * this->mtx[3] + other.y * this->mtx[7] + other.z * this->mtx[11] + other.w * this->mtx[15];

			return result;
		}

		void inverse() {
			const float xx = mtx[0];
			const float xy = mtx[1];
			const float xz = mtx[2];
			const float xw = mtx[3];
			const float yx = mtx[4];
			const float yy = mtx[5];
			const float yz = mtx[6];
			const float yw = mtx[7];
			const float zx = mtx[8];
			const float zy = mtx[9];
			const float zz = mtx[10];
			const float zw = mtx[11];
			const float wx = mtx[12];
			const float wy = mtx[13];
			const float wz = mtx[14];
			const float ww = mtx[15];

			float det = 0.0F;
			det += xx * (yy * (zz * ww - zw * wz) - yz * (zy * ww - zw * wy) + yw * (zy * wz - zz * wy));
			det -= xy * (yx * (zz * ww - zw * wz) - yz * (zx * ww - zw * wx) + yw * (zx * wz - zz * wx));
			det += xz * (yx * (zy * ww - zw * wy) - yy * (zx * ww - zw * wx) + yw * (zx * wy - zy * wx));
			det -= xw * (yx * (zy * wz - zz * wy) - yy * (zx * wz - zz * wx) + yz * (zx * wy - zy * wx));

			float invDet = 1.0F / det;

			this->mtx[0] = +(yy * (zz * ww - wz * zw) - yz * (zy * ww - wy * zw) + yw * (zy * wz - wy * zz)) * invDet;
			this->mtx[1] = -(xy * (zz * ww - wz * zw) - xz * (zy * ww - wy * zw) + xw * (zy * wz - wy * zz)) * invDet;
			this->mtx[2] = +(xy * (yz * ww - wz * yw) - xz * (yy * ww - wy * yw) + xw * (yy * wz - wy * yz)) * invDet;
			this->mtx[3] = -(xy * (yz * zw - zz * yw) - xz * (yy * zw - zy * yw) + xw * (yy * zz - zy * yz)) * invDet;

			this->mtx[4] = -(yx * (zz * ww - wz * zw) - yz * (zx * ww - wx * zw) + yw * (zx * wz - wx * zz)) * invDet;
			this->mtx[5] = +(xx * (zz * ww - wz * zw) - xz * (zx * ww - wx * zw) + xw * (zx * wz - wx * zz)) * invDet;
			this->mtx[6] = -(xx * (yz * ww - wz * yw) - xz * (yx * ww - wx * yw) + xw * (yx * wz - wx * yz)) * invDet;
			this->mtx[7] = +(xx * (yz * zw - zz * yw) - xz * (yx * zw - zx * yw) + xw * (yx * zz - zx * yz)) * invDet;

			this->mtx[8] = +(yx * (zy * ww - wy * zw) - yy * (zx * ww - wx * zw) + yw * (zx * wy - wx * zy)) * invDet;
			this->mtx[9] = -(xx * (zy * ww - wy * zw) - xy * (zx * ww - wx * zw) + xw * (zx * wy - wx * zy)) * invDet;
			this->mtx[10] = +(xx * (yy * ww - wy * yw) - xy * (yx * ww - wx * yw) + xw * (yx * wy - wx * yy)) * invDet;
			this->mtx[11] = -(xx * (yy * zw - zy * yw) - xy * (yx * zw - zx * yw) + xw * (yx * zy - zx * yy)) * invDet;

			this->mtx[12] = -(yx * (zy * wz - wy * zz) - yy * (zx * wz - wx * zz) + yz * (zx * wy - wx * zy)) * invDet;
			this->mtx[13] = +(xx * (zy * wz - wy * zz) - xy * (zx * wz - wx * zz) + xz * (zx * wy - wx * zy)) * invDet;
			this->mtx[14] = -(xx * (yy * wz - wy * yz) - xy * (yx * wz - wx * yz) + xz * (yx * wy - wx * yy)) * invDet;
			this->mtx[15] = +(xx * (yy * zz - zy * yz) - xy * (yx * zz - zx * yz) + xz * (yx * zy - zx * yy)) * invDet;
		}

		void lookAt(const rawrbox::Vector3f& _eye, const rawrbox::Vector3f& _at, const rawrbox::Vector3f& _up) {
			const rawrbox::Vector3f view = (_at - _eye).normalized();

			const rawrbox::Vector3f uxv = _up.cross(view);
			const rawrbox::Vector3f right = uxv.normalized();
			const rawrbox::Vector3f up = view.cross(right);

			this->mtx[0] = right.x;
			this->mtx[1] = up.x;
			this->mtx[2] = view.x;

			this->mtx[4] = right.y;
			this->mtx[5] = up.y;
			this->mtx[6] = view.y;

			this->mtx[8] = right.z;
			this->mtx[9] = up.z;
			this->mtx[10] = view.z;

			this->mtx[12] = -right.dot(_eye);
			this->mtx[13] = -up.dot(_eye);
			this->mtx[14] = -view.dot(_eye);
		}
		// ------

		// STATIC UTILS ----
		static inline rawrbox::Vector3f project(const rawrbox::Vector3f& pos, const rawrbox::Matrix4x4& view, const rawrbox::Matrix4x4& proj, const rawrbox::Vector4i& viewport) {
			std::array<float, 12> fTempo = {};
			// Modelview transform
			fTempo[0] = view[0] * pos.x + view[4] * pos.y + view[8] * pos.z + view[12]; // w is always 1
			fTempo[1] = view[1] * pos.x + view[5] * pos.y + view[9] * pos.z + view[13];
			fTempo[2] = view[2] * pos.x + view[6] * pos.y + view[10] * pos.z + view[14];
			fTempo[3] = view[3] * pos.x + view[7] * pos.y + view[11] * pos.z + view[15];

			// Projection transform, the final row of projection matrix is always [0 0 -1 0]
			// so we optimize for that.
			fTempo[4] = proj[0] * fTempo[0] + proj[4] * fTempo[1] + proj[8] * fTempo[2] + proj[12] * fTempo[3];
			fTempo[5] = proj[1] * fTempo[0] + proj[5] * fTempo[1] + proj[9] * fTempo[2] + proj[13] * fTempo[3];
			fTempo[6] = proj[2] * fTempo[0] + proj[6] * fTempo[1] + proj[10] * fTempo[2] + proj[14] * fTempo[3];
			fTempo[7] = -fTempo[2];

			// The result normalizes between -1 and 1
			if (fTempo[7] == 0.0F) // The w value
				return {};

			fTempo[7] = 1.0F / fTempo[7];

			// Perspective division
			fTempo[4] *= fTempo[7];
			fTempo[5] *= fTempo[7];
			fTempo[6] *= fTempo[7];

			// Map x, y to range 0-1
			rawrbox::Vector3f windowCoordinate = {};
			windowCoordinate.x = (fTempo[4] * -0.5F + 0.5F) * viewport.z + viewport.x;
			windowCoordinate.y = (fTempo[5] * 0.5F + 0.5F) * viewport.w + viewport.y;
			windowCoordinate.z = (1.0F + fTempo[6]) * 0.5F;

			return windowCoordinate;
		}

		// ------
		float operator[](size_t indx) const {
			return this->mtx[indx];
		}

		rawrbox::Matrix4x4 operator*(rawrbox::Matrix4x4 other) const {
			other.mul(*this);
			return other;
		}

		rawrbox::Matrix4x4 operator*(rawrbox::Vector3f other) const {
			rawrbox::Matrix4x4 res{this->data()};
			res.mul(other);

			return res;
		}

		bool operator==(const rawrbox::Matrix4x4& other) const {
			return std::equal(this->mtx.begin(), this->mtx.end(), other.mtx.begin());
		}

		bool operator!=(const rawrbox::Matrix4x4& other) const {
			return !operator==(other);
		}
	};
} // namespace rawrbox
