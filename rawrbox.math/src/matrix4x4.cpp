
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

namespace rawrbox {
	bool Matrix4x4::MTX_RIGHT_HANDED = false; // OpenGL / GLES

	// PRIVATE -----
	void Matrix4x4::vec4MulMtx(float* _result, const float* _vec, const float* _mat) {
		_result[0] = _vec[0] * _mat[0] + _vec[1] * _mat[4] + _vec[2] * _mat[8] + _vec[3] * _mat[12];
		_result[1] = _vec[0] * _mat[1] + _vec[1] * _mat[5] + _vec[2] * _mat[9] + _vec[3] * _mat[13];
		_result[2] = _vec[0] * _mat[2] + _vec[1] * _mat[6] + _vec[2] * _mat[10] + _vec[3] * _mat[14];
		_result[3] = _vec[0] * _mat[3] + _vec[1] * _mat[7] + _vec[2] * _mat[11] + _vec[3] * _mat[15];
	}

	void Matrix4x4::mul(const rawrbox::Matrix4x4& other) {
		rawrbox::Matrix4x4 _result;

		vec4MulMtx(_result.mtx.data(), this->mtx.data(), other.data());
		vec4MulMtx(&_result.mtx[4], &this->mtx[4], other.data());
		vec4MulMtx(&_result.mtx[8], &this->mtx[8], other.data());
		vec4MulMtx(&_result.mtx[12], &this->mtx[12], other.data());

		std::memcpy(this->mtx.data(), _result.data(), sizeof(float) * this->mtx.size());
	}

	void Matrix4x4::mul(const rawrbox::Vector3f& other) {
		rawrbox::Matrix4x4 _result;
		std::array<float, 4> m = {other.x, other.y, other.z, 1.F};

		vec4MulMtx(_result.mtx.data(), m.data(), this->data());
		vec4MulMtx(&_result.mtx[4], m.data(), this->data());
		vec4MulMtx(&_result.mtx[8], m.data(), this->data());
		vec4MulMtx(&_result.mtx[12], m.data(), this->data());

		std::memcpy(this->mtx.data(), _result.data(), sizeof(float) * this->mtx.size());
	}

	void Matrix4x4::add(const rawrbox::Matrix4x4& other) {
		for (size_t i = 0; i < this->mtx.size(); i++)
			this->mtx[i] += other[i];
	}

	void Matrix4x4::add(const rawrbox::Vector3f& other) {
		this->mtx[12] += other.x;
		this->mtx[13] += other.y;
		this->mtx[14] += other.z;
	}
	// -----------

	Matrix4x4::Matrix4x4(const std::array<float, 16>& other) { std::memcpy(this->mtx.data(), other.data(), sizeof(float) * this->mtx.size()); };
	Matrix4x4::Matrix4x4(const float* other) { std::memcpy(this->mtx.data(), other, sizeof(float) * this->mtx.size()); };

	//  -------
	size_t Matrix4x4::size() const { return this->mtx.size(); }
	const float* Matrix4x4::data() const { return this->mtx.data(); }
	float* Matrix4x4::data() { return this->mtx.data(); }
	// ---

	// UTILS ----
	rawrbox::Matrix4x4& Matrix4x4::zero() {
		std::memset(this->mtx.data(), 0, sizeof(float) * this->mtx.size());
		return *this;
	}

	rawrbox::Matrix4x4& Matrix4x4::identity() {
		this->mtx = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
		return *this;
	}

	rawrbox::Matrix4x4& Matrix4x4::transpose(const float* other) {
		this->mtx = {
		    other[0], other[4], other[8], other[12],
		    other[1], other[5], other[9], other[13],
		    other[2], other[6], other[10], other[14],
		    other[3], other[7], other[11], other[15]};

		return *this;
	}
	rawrbox::Matrix4x4& Matrix4x4::transpose() {
		this->transpose(this->data());
		return *this;
	}

	rawrbox::Matrix4x4& Matrix4x4::transpose(const Matrix4x4& other) {
		this->transpose(other.data());
		return *this;
	}

	rawrbox::Matrix4x4& Matrix4x4::translate(const rawrbox::Vector3f& pos) {
		this->mtx[12] = pos.x;
		this->mtx[13] = pos.y;
		this->mtx[14] = pos.z;
		return *this;
	}

	rawrbox::Matrix4x4& Matrix4x4::scale(const rawrbox::Vector3f& scale) {
		this->mtx[0] = scale.x;
		this->mtx[5] = scale.y;
		this->mtx[10] = scale.z;

		return *this;
	}

	rawrbox::Vector3f Matrix4x4::getPos() const {
		return {this->mtx[12], this->mtx[13], this->mtx[14]};
	}

	rawrbox::Vector3f Matrix4x4::getScale() const {
		return {this->mtx[0], this->mtx[5], this->mtx[10]};
	}

	rawrbox::Vector4f Matrix4x4::getRotation() const {
		rawrbox::Vector4f result = {};

		float trace = this->mtx[0] + this->mtx[5] + this->mtx[10];
		if (trace > 0) {
			float s = 0.5f / sqrtf(trace + 1.0f);
			result.w = 0.25f / s;
			result.x = (this->mtx[9] - this->mtx[6]) * s;
			result.y = (this->mtx[2] - this->mtx[8]) * s;
			result.z = (this->mtx[4] - this->mtx[1]) * s;
		} else {
			if (this->mtx[0] > this->mtx[5] && this->mtx[0] > this->mtx[10]) {
				float s = 2.0f * sqrtf(1.0f + this->mtx[0] - this->mtx[5] - this->mtx[10]);
				result.w = (this->mtx[9] - this->mtx[6]) / s;
				result.x = 0.25f * s;
				result.y = (this->mtx[1] + this->mtx[4]) / s;
				result.z = (this->mtx[2] + this->mtx[8]) / s;
			} else if (this->mtx[5] > this->mtx[10]) {
				float s = 2.0f * sqrtf(1.0f + this->mtx[5] - this->mtx[0] - this->mtx[10]);
				result.w = (this->mtx[2] - this->mtx[8]) / s;
				result.x = (this->mtx[1] + this->mtx[4]) / s;
				result.y = 0.25f * s;
				result.z = (this->mtx[6] + this->mtx[9]) / s;
			} else {
				float s = 2.0f * sqrtf(1.0f + this->mtx[10] - this->mtx[0] - this->mtx[5]);
				result.w = (this->mtx[4] - this->mtx[1]) / s;
				result.x = (this->mtx[2] + this->mtx[8]) / s;
				result.y = (this->mtx[6] + this->mtx[9]) / s;
				result.z = 0.25f * s;
			}
		}

		result.w = -result.w; // ??
		return result;
	}

	void Matrix4x4::decompose(rawrbox::Vector3f& pos, rawrbox::Vector4f& rotation, rawrbox::Vector3f& scale) const {
		pos = this->getPos();
		rotation = this->getRotation();
		scale = this->getScale();
	}

	rawrbox::Matrix4x4& Matrix4x4::rotate(const rawrbox::Vector4f& rot) {
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

		return *this;
	}

	rawrbox::Matrix4x4& Matrix4x4::rotateX(float x) {
		const float sx = sin(x);
		const float cx = cos(x);

		this->mtx[5] = cx;
		this->mtx[6] = -sx;
		this->mtx[9] = sx;
		this->mtx[10] = cx;

		return *this;
	}

	rawrbox::Matrix4x4& Matrix4x4::rotateY(float y) {
		const float sy = sin(y);
		const float cy = cos(y);

		this->mtx[0] = cy;
		this->mtx[2] = sy;
		this->mtx[8] = -sy;
		this->mtx[10] = cy;

		return *this;
	}

	rawrbox::Matrix4x4& Matrix4x4::rotateZ(float z) {
		const float sz = sin(z);
		const float cz = cos(z);

		this->mtx[0] = cz;
		this->mtx[1] = -sz;
		this->mtx[4] = sz;
		this->mtx[5] = cz;

		return *this;
	}

	rawrbox::Matrix4x4& Matrix4x4::rotateXYZ(const rawrbox::Vector3f& rot) {
		this->rotateX(rot.x);
		this->rotateY(rot.y);
		this->rotateZ(rot.z);

		return *this;
	}

	// Views
	rawrbox::Matrix4x4& Matrix4x4::ortho(float left, float right, float bottom, float top, float near, float far) {
		this->mtx[0] = 2.0F / (right - left);
		this->mtx[5] = 2.0F / (top - bottom);

		if (Matrix4x4::MTX_RIGHT_HANDED) {
			this->mtx[10] = -2.0F / (far - near);
		} else {
			this->mtx[10] = 2.0F / (far - near);
		}

		this->mtx[12] = -(right + left) / (right - left);
		this->mtx[13] = -(top + bottom) / (top - bottom);
		this->mtx[14] = -(far + near) / (far - near);

		return *this;
	}

	rawrbox::Matrix4x4& Matrix4x4::proj(float FOV, float aspect, float near, float far) {
		this->zero();

		float const tanHalfFovy = std::tan(rawrbox::MathUtils::toRad(FOV) / 2.F);
		this->mtx[0] = 1.F / (aspect * tanHalfFovy);
		this->mtx[5] = 1.F / (tanHalfFovy);
		this->mtx[14] = -(2.F * far * near) / (far - near);
		this->mtx[15] = 0.F;

		if (Matrix4x4::MTX_RIGHT_HANDED) {
			this->mtx[10] = -(far + near) / (far - near);
			this->mtx[11] = -1.F;
		} else {
			this->mtx[10] = (far + near) / (far - near);
			this->mtx[11] = 1.F;
		}

		return *this;
	}

	rawrbox::Matrix4x4& Matrix4x4::billboard(const rawrbox::Matrix4x4& view, const rawrbox::Vector3f& pos, const rawrbox::Vector3f& scale) {
		this->mtx[0] = view[0] * scale.x;
		this->mtx[1] = view[4] * scale.x;
		this->mtx[2] = view[8] * scale.x;
		this->mtx[3] = 0.0F;
		this->mtx[4] = view[1] * scale.y;
		this->mtx[5] = view[5] * scale.y;
		this->mtx[6] = view[9] * scale.y;
		this->mtx[7] = 0.0F;
		this->mtx[8] = view[2] * scale.z;
		this->mtx[9] = view[6] * scale.z;
		this->mtx[10] = view[10] * scale.z;
		this->mtx[11] = 0.0F;
		this->mtx[12] = pos.x;
		this->mtx[13] = pos.y;
		this->mtx[14] = pos.z;
		this->mtx[15] = 1.0F;

		return *this;
	}
	// ----------

	rawrbox::Matrix4x4& Matrix4x4::SRT(const rawrbox::Vector3f& scale, const rawrbox::Vector4f& rotation, const rawrbox::Vector3f& pos) {
		rawrbox::Matrix4x4 mt = {};
		mt.translate(pos);

		rawrbox::Matrix4x4 ms = {};
		ms.scale(scale);

		rawrbox::Matrix4x4 mr = {};
		mr.rotate(rotation); // Angle should be in world coords

		this->mtx = (mt * mr * ms).mtx;
		return *this;
	}

	[[nodiscard]] rawrbox::Vector3f Matrix4x4::mulVec(const rawrbox::Vector3f& other) const {
		rawrbox::Vector3f result = {};

		result.x = other.x * this->mtx[0] + other.y * this->mtx[4] + other.z * this->mtx[8] + this->mtx[12];
		result.y = other.x * this->mtx[1] + other.y * this->mtx[5] + other.z * this->mtx[9] + this->mtx[13];
		result.z = other.x * this->mtx[2] + other.y * this->mtx[6] + other.z * this->mtx[10] + this->mtx[14];

		return result;
	}

	[[nodiscard]] rawrbox::Vector4f Matrix4x4::mulVec(const rawrbox::Vector4f& other) const {
		rawrbox::Vector4f result = {};

		result.x = other.x * this->mtx[0] + other.y * this->mtx[4] + other.z * this->mtx[8] + other.w * this->mtx[12];
		result.y = other.x * this->mtx[1] + other.y * this->mtx[5] + other.z * this->mtx[9] + other.w * this->mtx[13];
		result.z = other.x * this->mtx[2] + other.y * this->mtx[6] + other.z * this->mtx[10] + other.w * this->mtx[14];
		result.w = other.x * this->mtx[3] + other.y * this->mtx[7] + other.z * this->mtx[11] + other.w * this->mtx[15];

		return result;
	}

	rawrbox::Matrix4x4& Matrix4x4::inverse() {
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

		return *this;
	}

	rawrbox::Matrix4x4& Matrix4x4::lookAt(const rawrbox::Vector3f& _eye, const rawrbox::Vector3f& _at, const rawrbox::Vector3f& _up) {
		rawrbox::Vector3f const f = (Matrix4x4::MTX_RIGHT_HANDED ? _eye - _at : _at - _eye).normalized();
		rawrbox::Vector3f const s = _up.cross(f).normalized();
		rawrbox::Vector3f const u = f.cross(s);

		this->mtx[0] = s.x;
		this->mtx[4] = s.y;
		this->mtx[8] = s.z;
		this->mtx[1] = u.x;
		this->mtx[5] = u.y;
		this->mtx[9] = u.z;
		this->mtx[2] = f.x;
		this->mtx[6] = f.y;
		this->mtx[10] = f.z;
		this->mtx[12] = -s.dot(_eye);
		this->mtx[13] = -u.dot(_eye);
		this->mtx[14] = -f.dot(_eye);

		return *this;
	}
	// ------

	// STATIC UTILS ----
	rawrbox::Matrix4x4 Matrix4x4::mtxSRT(const rawrbox::Vector3f& scale, const rawrbox::Vector4f& rotation, const rawrbox::Vector3f& pos) {
		rawrbox::Matrix4x4 ret = {};
		ret.SRT(scale, rotation, pos);

		return ret;
	}

	rawrbox::Matrix4x4 Matrix4x4::mtxTranspose(rawrbox::Matrix4x4 mtx) {
		mtx.transpose();
		return mtx;
	}

	rawrbox::Matrix4x4 Matrix4x4::mtxInverse(rawrbox::Matrix4x4 mtx) {
		mtx.inverse();
		return mtx;
	}

	rawrbox::Matrix4x4 Matrix4x4::mtxLookAt(const rawrbox::Vector3f& _eye, const rawrbox::Vector3f& _at, const rawrbox::Vector3f& _up) {
		rawrbox::Matrix4x4 ret = {};
		ret.lookAt(_eye, _at, _up);

		return ret;
	}

	// Create a ortographic projection
	rawrbox::Matrix4x4 Matrix4x4::mtxOrtho(float left, float right, float bottom, float top, float near, float far) {
		rawrbox::Matrix4x4 ret = {};
		ret.ortho(left, right, bottom, top, near, far);

		return ret;
	}

	// Create a projection matrix
	rawrbox::Matrix4x4 Matrix4x4::mtxProj(float FOV, float aspect, float near, float far) {
		rawrbox::Matrix4x4 ret = {};
		ret.proj(FOV, aspect, near, far);

		return ret;
	}

	rawrbox::Vector3f Matrix4x4::mtxProject(const rawrbox::Vector3f& pos, const rawrbox::Matrix4x4& view, const rawrbox::Matrix4x4& proj, const rawrbox::Vector4u& viewport) {
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

	// OPERATORS ----
	float Matrix4x4::operator[](size_t indx) const {
		return this->mtx[indx];
	}

	float Matrix4x4::operator[](size_t indx) {
		return this->mtx[indx];
	}

	void Matrix4x4::operator/=(float other) {
		for (float& i : this->mtx) {
			i /= other;
		}
	}

	void Matrix4x4::operator/=(rawrbox::Matrix4x4 other) {
		other.inverse();
		this->mul(other);
	}

	void Matrix4x4::operator*=(const rawrbox::Matrix4x4& other) {
		this->mul(other);
	}

	void Matrix4x4::operator*=(const rawrbox::Vector3f& other) {
		this->mul(other);
	}

	rawrbox::Matrix4x4 Matrix4x4::operator*(rawrbox::Matrix4x4 other) const {
		other.mul(*this);
		return other;
	}

	rawrbox::Matrix4x4 Matrix4x4::operator*(rawrbox::Vector3f other) const {
		rawrbox::Matrix4x4 res = *this;
		res.mul(other);

		return res;
	}

	rawrbox::Matrix4x4 Matrix4x4::operator+(rawrbox::Matrix4x4 other) const {
		other.add(*this);
		return other;
	}

	rawrbox::Matrix4x4 Matrix4x4::operator+(rawrbox::Vector3f other) const {
		rawrbox::Matrix4x4 res = *this;
		res.add(other);

		return res;
	}

	rawrbox::Matrix4x4 Matrix4x4::operator/(rawrbox::Matrix4x4 other) const {
		other.inverse();
		other.mul(*this);
		return other;
	}

	bool Matrix4x4::operator==(const rawrbox::Matrix4x4& other) const {
		return std::equal(this->mtx.begin(), this->mtx.end(), other.mtx.begin());
	}

	bool Matrix4x4::operator!=(const rawrbox::Matrix4x4& other) const {
		return !operator==(other);
	}
	// ------

} // namespace rawrbox
