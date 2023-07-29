#pragma once

#include <rawrbox/math/pi.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <type_traits>

namespace rawrbox {
	template <class NumberType>
	class Vector4_t {
	protected:
		using VecType = Vector4_t<NumberType>;

	public:
		NumberType x = 0, y = 0, z = 0, w = 0;

		Vector4_t() = default;
		explicit Vector4_t(NumberType val) : x(val), y(val), z(val), w(val) {}

		Vector4_t(NumberType _x, NumberType _y, NumberType _z, NumberType _w = 0.F) : x(_x), y(_y), z(_z), w(_w) {}

		// NOLINTBEGIN(hicpp-explicit-conversions)
		Vector4_t(std::array<NumberType, 4> val) : x(val[0]), y(val[1]), z(val[2]), w(val[3]) {}
		Vector4_t(Vector3_t<NumberType> val, NumberType _w = 0.F) : x(val.x), y(val.y), z(val.z), w(_w) {}
		Vector4_t(Vector2_t<NumberType> val, NumberType _z = 0.F, NumberType _w = 0.F) : x(val.x), y(val.y), z(_z), w(_w) {}
		// NOLINTEND(hicpp-explicit-conversions)

		static VecType zero() { return VecType(); }
		static VecType one() { return VecType(1, 1, 1, 1); }
		[[nodiscard]] int size() const { return 4; }

		[[nodiscard]] Vector3_t<NumberType> xyz() const { return Vector3_t<NumberType>(x, y, z); }
		[[nodiscard]] Vector3_t<NumberType> yxz() const { return Vector3_t<NumberType>(y, x, z); }
		[[nodiscard]] Vector3_t<NumberType> yzx() const { return Vector3_t<NumberType>(y, z, x); }
		[[nodiscard]] Vector3_t<NumberType> xzy() const { return Vector3_t<NumberType>(x, z, y); }
		[[nodiscard]] Vector3_t<NumberType> zxy() const { return Vector3_t<NumberType>(z, x, y); }
		[[nodiscard]] Vector3_t<NumberType> zyx() const { return Vector3_t<NumberType>(z, y, x); }

		[[nodiscard]] Vector2_t<NumberType> xy() const { return Vector2_t<NumberType>(x, y); }
		[[nodiscard]] Vector2_t<NumberType> yx() const { return Vector2_t<NumberType>(y, x); }

		[[nodiscard]] const std::array<NumberType, 4> data() const { return {x, y, z, w}; }

		NumberType length() const {
			return static_cast<NumberType>(std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2) + std::pow(w, 2)));
		}

		NumberType lengthSquared() const {
			return static_cast<NumberType>(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2) + std::pow(w, 2));
		}

		VecType normalized() const {
			return (*this) / length();
		}

		[[nodiscard]] VecType clamp(NumberType min, NumberType max) const {
			return {
			    std::clamp(x, min, max),
			    std::clamp(y, min, max),
			    std::clamp(z, min, max),
			    std::clamp(w, min, max)};
		}

		[[nodiscard]] VecType clamp(VecType min, VecType max) const {
			return {
			    std::clamp(x, min.x, max.x),
			    std::clamp(y, min.y, max.y),
			    std::clamp(z, min.z, max.z),
			    std::clamp(w, min.w, max.w)};
		}

		VecType lerp(const VecType& other, NumberType timestep) const {
			if ((*this) == other) return other;
			VecType ret;

			float dot = x * other.x + y * other.y + z * other.z + w * other.w;
			float blend = 1.F - timestep;

			if (dot < 0.F) {
				ret.x = blend * x + blend * -other.x;
				ret.y = blend * y + blend * -other.y;
				ret.z = blend * z + blend * -other.z;
				ret.w = blend * w + blend * -other.w;
			} else {
				ret.x = blend * x + blend * other.x;
				ret.y = blend * y + blend * other.y;
				ret.z = blend * z + blend * other.z;
				ret.w = blend * w + blend * other.w;
			}

			return ret.normalized();
		}

		Vector3_t<NumberType> toEuler() const {
			// roll
			float sinr_cosp = 2.0F * (w * x + y * z);
			float cosr_cosp = 1.0F - 2.0F * (x * x + y * y);

			// pitch
			float sinp = 2.0F * (w * y - z * x);
			sinp = sinp > 1.0F ? 1.0F : sinp;
			sinp = sinp < -1.0F ? -1.0F : sinp;

			// yaw
			float siny_cosp = 2.0F * (w * z + x * y);
			float cosy_cosp = 1.0F - 2.0F * (y * y + z * z);

			return {std::atan2(sinr_cosp, cosr_cosp), std::asin(sinp), std::atan2(siny_cosp, cosy_cosp)};
		}

		Vector3_t<NumberType> toAxis() {
			if (std::abs(this->w) > 1.0F) {
				auto nrm = this->normalized();

				this->x = nrm.x;
				this->y = nrm.y;
				this->z = nrm.z;
				this->w = nrm.w;
			}

			float den = std::sqrt(1.0F - this->w * this->w);
			if (den > 0.0001F) return {this->x / den, this->y / den, this->z / den};
			return {1, 0, 0};
		}

		static Vector4_t<NumberType> toQuat(const Vector3_t<NumberType>& in) {
			rawrbox::Vector4_t<NumberType> ret = {};

			float x = in.x * 0.5F;
			float y = in.y * -0.5F;
			float z = in.z * 0.5F;

			float sinX = std::sin(x);
			float cosX = std::cos(x);
			float sinY = std::sin(y);
			float cosY = std::cos(y);
			float sinZ = std::sin(z);
			float cosZ = std::cos(z);

			ret.w = cosY * cosX * cosZ + sinY * sinX * sinZ;
			ret.x = cosY * sinX * cosZ + sinY * cosX * sinZ;
			ret.y = sinY * cosX * cosZ - cosY * sinX * sinZ;
			ret.z = cosY * cosX * sinZ - sinY * sinX * cosZ;

			return ret;
		}

		static Vector4_t<NumberType> lookRotation(const Vector3_t<NumberType>& lookAt, const Vector3_t<NumberType>& upDirection) {
			auto forward = lookAt.normalized();
			auto right = upDirection.cross(forward).normalized();
			auto up = forward.cross(right);

			auto m00 = right.x;
			auto m01 = right.y;
			auto m02 = right.z;
			auto m10 = up.x;
			auto m11 = up.y;
			auto m12 = up.z;
			auto m20 = forward.x;
			auto m21 = forward.y;
			auto m22 = forward.z;

			float num8 = (m00 + m11) + m22;
			rawrbox::Vector4_t<NumberType> ret = {};

			if (num8 > 0.F) {
				auto num = std::sqrt(num8 + 1.F);

				ret.w = num * 0.5F;
				num = 0.5F / num;

				ret.x = (m12 - m21) * num;
				ret.y = (m20 - m02) * num;
				ret.z = (m01 - m10) * num;
				return ret;
			}

			if ((m00 >= m11) && (m00 >= m22)) {
				auto num7 = std::sqrt(((1.F + m00) - m11) - m22);
				auto num4 = 0.5F / num7;

				ret.x = 0.5F * num7;
				ret.y = (m01 + m10) * num4;
				ret.z = (m02 + m20) * num4;
				ret.w = (m12 - m21) * num4;
				return ret;
			}

			if (m11 > m22) {
				auto num6 = std::sqrt(((1.F + m11) - m00) - m22);
				auto num3 = 0.5F / num6;

				ret.x = (m10 + m01) * num3;
				ret.y = 0.5F * num6;
				ret.z = (m21 + m12) * num3;
				ret.w = (m20 - m02) * num3;
				return ret;
			}

			auto num5 = std::sqrt(((1.F + m22) - m00) - m11);
			auto num2 = 0.5F / num5;

			ret.x = (m20 + m02) * num2;
			ret.y = (m21 + m12) * num2;
			ret.z = 0.5F * num5;
			ret.w = (m01 - m10) * num2;

			return ret;
		}

		VecType inverse() {
			float lengthSq = this->lengthSquared();
			if (lengthSq == 0.0F) return *this;

			float i = 1.0F / lengthSq;
			return VecType(this->xyz() * -i, this->w * i);
		}

		VecType interpolate(const VecType& pEnd, float pFactor) {
			// calc cosine theta
			float cosom = x * pEnd.x + y * pEnd.y + z * pEnd.z + w * pEnd.w;

			// adjust signs (if necessary)
			VecType end = pEnd;
			if (cosom < static_cast<float>(0.0)) {
				cosom = -cosom;
				end.x = -end.x; // Reverse all signs
				end.y = -end.y;
				end.z = -end.z;
				end.w = -end.w;
			}

			// Calculate coefficients
			// NOLINTBEGIN(clang-analyzer-deadcode.DeadStores)
			float sclp = NAN;
			float sclq = NAN;

			if ((static_cast<float>(1.0) - cosom) > static_cast<float>(0.0001)) // 0.0001 -> some epsillon
			{
				// Standard case (slerp)
				float omega = NAN, sinom = NAN;
				omega = std::acos(cosom); // extract theta from dot product's cos theta
				sinom = std::sin(omega);
				sclp = std::sin((static_cast<float>(1.0) - pFactor) * omega) / sinom;
				sclq = std::sin(pFactor * omega) / sinom;
			} else {
				// Very close, do linear interp (because it's faster)
				sclp = static_cast<float>(1.0) - pFactor;
				sclq = pFactor;
			}
			// NOLINTEND(clang-analyzer-deadcode.DeadStores)

			VecType pOut;
			pOut.x = sclp * x + sclq * end.x;
			pOut.y = sclp * y + sclq * end.y;
			pOut.z = sclp * z + sclq * end.z;
			pOut.w = sclp * w + sclq * end.w;

			return pOut;
		};

		// OPERATORS ---
		// numberic typed operators
		VecType operator-(NumberType other) const { return VecType(x - other, y - other, z - other, w - other); }
		VecType operator+(NumberType other) const { return VecType(x + other, y + other, z + other, w + other); }
		VecType operator*(NumberType other) const { return VecType(x * other, y * other, z * other, w * other); }
		VecType operator/(NumberType other) const { return VecType(x / other, y / other, z / other, w / other); }

		VecType& operator-=(NumberType other) {
			x = x - other;
			y = y - other;
			z = z - other;
			w = w - other;
			return *this;
		}

		VecType& operator+=(NumberType other) {
			x = x + other;
			y = y + other;
			z = z + other;
			w = w + other;
			return *this;
		}

		VecType& operator*=(NumberType other) {
			x = x * other;
			y = y * other;
			z = z * other;
			w = w * other;
			return *this;
		}

		VecType& operator/=(NumberType other) {
			x = x / other;
			y = y / other;
			z = z / other;
			w = w / other;
			return *this;
		}

		bool operator==(NumberType other) const { return x == other && y == other && z == other && w == other; }
		bool operator!=(NumberType other) const { return !operator==(other); }
		bool operator<(NumberType other) const { return x < other && y < other && z < other && w < other; }
		bool operator<=(NumberType other) const { return x <= other && y <= other && z <= other && w <= other; }
		bool operator>(NumberType other) const { return x > other && y > other && z > other && w > other; }
		bool operator>=(NumberType other) const { return x >= other && y >= other && z >= other && w >= other; }

		// vector typed operators
		VecType operator-(const VecType& other) const { return VecType(x - other.x, y - other.y, z - other.z, w - other.w); }
		VecType operator+(const VecType& other) const { return VecType(x + other.x, y + other.y, z + other.z, w + other.w); }
		VecType operator/(const VecType& other) const { return VecType(x / other.x, y / other.y, z / other.z, w / other.w); }
		VecType operator*(const VecType& other) const { return VecType(this->w * other.x + this->x * other.w + this->y * other.z - this->z * other.y, this->w * other.y + this->y * other.w + this->z * other.x - this->x * other.z, this->w * other.z + this->z * other.w + this->x * other.y - this->y * other.x, this->w * other.w - this->x * other.x - this->y * other.y - this->z * other.z); }

		VecType& operator-=(const VecType& other) {
			x = x - other.x;
			y = y - other.y;
			z = z - other.z;
			w = w - other.w;
			return *this;
		}

		VecType& operator+=(const VecType& other) {
			x = x + other.x;
			y = y + other.y;
			z = z + other.z;
			w = w + other.w;
			return *this;
		}

		VecType& operator*=(const VecType& other) const {
			x = this->w * other.x + this->x * other.w + this->y * other.z - this->z * other.y;
			y = this->w * other.y + this->y * other.w + this->z * other.x - this->x * other.z;
			z = this->w * other.z + this->z * other.w + this->x * other.y - this->y * other.x;
			w = this->w * other.w - this->x * other.x - this->y * other.y - this->z * other.z;
			return *this;
		}

		VecType& operator/=(const VecType& other) {
			x = x / other.x;
			y = y / other.y;
			z = z / other.z;
			w = w / other.w;
			return *this;
		}

		bool operator==(const VecType& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
		bool operator!=(const VecType& other) const { return !operator==(other); }
		bool operator<(const VecType& other) const { return x < other.x && y < other.y && z < other.z && w < other.w; }
		bool operator<=(const VecType& other) const { return x <= other.x && y <= other.y && z <= other.z && w <= other.w; }
		bool operator>(const VecType& other) const { return x > other.x && y > other.y && z > other.z && w > other.w; }
		bool operator>=(const VecType& other) const { return x >= other.x && y >= other.y && z >= other.z && w >= other.w; }

		VecType operator-() const { return VecType(-x, -y, -z, -w); }

		Vector3_t<NumberType> operator*(const Vector3_t<NumberType>& other) const {
			float num = this->x * 2.F;
			float num2 = this->y * 2.F;
			float num3 = this->z * 2.F;
			float num4 = this->x * num;
			float num5 = this->y * num2;
			float num6 = this->z * num3;
			float num7 = this->x * num2;
			float num8 = this->x * num3;
			float num9 = this->y * num3;
			float num10 = this->w * num;
			float num11 = this->w * num2;
			float num12 = this->w * num3;

			Vector3_t<NumberType> result = {};
			result.x = (1.F - (num5 + num6)) * other.x + (num7 - num12) * other.y + (num8 + num11) * other.z;
			result.y = (num7 + num12) * other.x + (1.F - (num4 + num6)) * other.y + (num9 - num10) * other.z;
			result.z = (num8 - num11) * other.x + (num9 + num10) * other.y + (1.F - (num4 + num5)) * other.z;
			return result;
		}
		// -------
	};

	using Vector4f = Vector4_t<float>;
	using Vector4i = Vector4_t<int>;
	using Vector4 = Vector4f;
} // namespace rawrbox
