#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>

#include <algorithm>
#include <array>
#include <cmath>

namespace rawrbox {

	template <class NumberType>
		requires(std::is_integral_v<NumberType> || std::is_floating_point_v<NumberType>)
	class Vector4_t {
	protected:
		using VecType = Vector4_t<NumberType>;

	public:
		NumberType x = 0, y = 0, z = 0, w = 0;
		Vector4_t() = default;
		constexpr explicit Vector4_t(NumberType val) : x(val), y(val), z(val), w(val) {}
		constexpr Vector4_t(NumberType _x, NumberType _y, NumberType _z = 0, NumberType _w = 0) : x(_x), y(_y), z(_z), w(_w) {}

		// NOLINTBEGIN(hicpp-explicit-conversions)
		constexpr Vector4_t(const std::array<NumberType, 4>& val) : x(val[0]), y(val[1]), z(val[2]), w(val[3]) {}
		constexpr Vector4_t(Vector3_t<NumberType> val, NumberType _w = 0) : x(val.x), y(val.y), z(val.z), w(_w) {}
		constexpr Vector4_t(Vector2_t<NumberType> val, NumberType _z = 0, NumberType _w = 0) : x(val.x), y(val.y), z(_z), w(_w) {}
		// NOLINTEND(hicpp-explicit-conversions)

		static VecType nan()
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{ return VecType(std::numeric_limits<NumberType>::quiet_NaN(), std::numeric_limits<NumberType>::quiet_NaN(), std::numeric_limits<NumberType>::quiet_NaN(), std::numeric_limits<NumberType>::quiet_NaN()); }

		[[nodiscard]] bool isNAN() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{ return std::isnan<NumberType>(this->x) && std::isnan<NumberType>(this->y) && std::isnan<NumberType>(this->z) && std::isnan<NumberType>(this->w); }

		static VecType zero() { return VecType(); }
		static VecType one() { return VecType(1, 1, 1, 1); }
		[[nodiscard]] int size() const { return 4; }
		[[nodiscard]] std::string toString() const { return std::to_string(this->x) + "," + std::to_string(this->y) + "," + std::to_string(this->z) + "," + std::to_string(this->w); }

		// TODO: ADD VEC4 VARIANTS

		[[nodiscard]] Vector3_t<NumberType> xyz() const { return Vector3_t<NumberType>(this->x, this->y, this->z); }
		[[nodiscard]] Vector3_t<NumberType> yxz() const { return Vector3_t<NumberType>(this->y, this->x, this->z); }
		[[nodiscard]] Vector3_t<NumberType> yzx() const { return Vector3_t<NumberType>(this->y, this->z, this->x); }
		[[nodiscard]] Vector3_t<NumberType> xzy() const { return Vector3_t<NumberType>(this->x, this->z, this->y); }
		[[nodiscard]] Vector3_t<NumberType> zxy() const { return Vector3_t<NumberType>(this->z, this->x, this->y); }
		[[nodiscard]] Vector3_t<NumberType> zyx() const { return Vector3_t<NumberType>(this->z, this->y, this->x); }

		[[nodiscard]] Vector2_t<NumberType> xy() const { return Vector2_t<NumberType>(this->x, this->y); }
		[[nodiscard]] Vector2_t<NumberType> yx() const { return Vector2_t<NumberType>(this->y, this->x); }
		[[nodiscard]] Vector2_t<NumberType> xz() const { return Vector2_t<NumberType>(this->x, this->z); }
		[[nodiscard]] Vector2_t<NumberType> yz() const { return Vector2_t<NumberType>(this->y, this->z); }

		[[nodiscard]] Vector2_t<NumberType> zx() const { return Vector2_t<NumberType>(this->z, this->x); }
		[[nodiscard]] Vector2_t<NumberType> zy() const { return Vector2_t<NumberType>(this->z, this->y); }
		[[nodiscard]] Vector2_t<NumberType> zw() const { return Vector2_t<NumberType>(this->z, this->w); }

		[[nodiscard]] std::array<NumberType, 4> data() const { return {this->x, this->y, this->z, this->w}; }

		NumberType length() const {
			return static_cast<NumberType>(std::sqrt(std::pow(this->x, 2) + std::pow(this->y, 2) + std::pow(this->z, 2) + std::pow(this->w, 2)));
		}

		NumberType sqrMagnitude() const {
			return static_cast<NumberType>(std::pow(this->x, 2) + std::pow(this->y, 2) + std::pow(this->z, 2) + std::pow(this->w, 2));
		}

		VecType normalized() const {
			return (*this) / this->length();
		}

		template <class ReturnType>
			requires(!std::is_same_v<NumberType, ReturnType>)
		Vector4_t<ReturnType> cast() const {
			return {static_cast<ReturnType>(this->x), static_cast<ReturnType>(this->y), static_cast<ReturnType>(this->z), static_cast<ReturnType>(this->w)};
		}

		[[nodiscard]] VecType clamp(NumberType min, NumberType max) const {
			return {
			    std::clamp(this->x, min, max),
			    std::clamp(this->y, min, max),
			    std::clamp(this->z, min, max),
			    std::clamp(this->w, min, max)};
		}

		[[nodiscard]] VecType clamp(VecType min, VecType max) const {
			return {
			    std::clamp(this->x, min.x, max.x),
			    std::clamp(this->y, min.y, max.y),
			    std::clamp(this->z, min.z, max.z),
			    std::clamp(this->w, min.w, max.w)};
		}

		VecType lerp(const VecType& other, NumberType timestep) const
			requires(!std::is_same_v<NumberType, uint16_t>)
		{
			VecType ret;

			ret.x = static_cast<NumberType>(static_cast<float>(this->x) + static_cast<float>(other.x - this->x) * timestep);
			ret.y = static_cast<NumberType>(static_cast<float>(this->y) + static_cast<float>(other.y - this->y) * timestep);
			ret.z = static_cast<NumberType>(static_cast<float>(this->z) + static_cast<float>(other.z - this->z) * timestep);
			ret.w = static_cast<NumberType>(static_cast<float>(this->w) + static_cast<float>(other.w - this->w) * timestep);
			return ret;
		}

		VecType slerp(const VecType& other, NumberType timestep) const
			requires(!std::is_same_v<NumberType, uint16_t>)
		{
			VecType ret;

			auto lerpSingleAngle = [](NumberType a, NumberType b, NumberType t) {
				NumberType difference = std::fmod(b - a, TWO<NumberType> * rawrbox::pi<NumberType>);
				if (difference > static_cast<NumberType>(rawrbox::pi<NumberType>))
					difference -= 2.0F * static_cast<NumberType>(rawrbox::pi<NumberType>);
				else if (difference < -static_cast<NumberType>(rawrbox::pi<NumberType>))
					difference += 2.0F * static_cast<NumberType>(rawrbox::pi<NumberType>);
				return a + difference * t;
			};

			ret.x = lerpSingleAngle(this->x, other.x, timestep);
			ret.y = lerpSingleAngle(this->y, other.y, timestep);
			ret.z = lerpSingleAngle(this->z, other.z, timestep);
			ret.w = lerpSingleAngle(this->w, other.w, timestep);

			return ret;
		}

		// FLOATING NUMBERS ----
		[[nodiscard]] std::array<uint16_t, 4> pack() const
			requires(std::is_same_v<NumberType, float>)
		{
			uint16_t xx = rawrbox::PackUtils::toFP16(this->x);
			uint16_t yy = rawrbox::PackUtils::toFP16(this->y);
			uint16_t zz = rawrbox::PackUtils::toFP16(this->z);
			uint16_t ww = rawrbox::PackUtils::toFP16(this->w);

			return {xx, yy, zz, ww};
		}

		// UTILS uint16_t --
		[[nodiscard]] std::array<float, 4> unpack() const
			requires(std::is_same_v<NumberType, uint16_t>)
		{
			float xx = rawrbox::PackUtils::fromFP16(this->x);
			float yy = rawrbox::PackUtils::fromFP16(this->y);
			float zz = rawrbox::PackUtils::fromFP16(this->z);
			float ww = rawrbox::PackUtils::fromFP16(this->w);

			return {xx, yy, zz, ww};
		}
		// ----------

		Vector3_t<NumberType> toEuler() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			// roll
			NumberType sinr_cosp = TWO<NumberType> * (this->w * this->x + this->y * this->z);
			NumberType cosr_cosp = ONE<NumberType> - TWO<NumberType> * (this->x * this->x + this->y * this->y);

			// pitch
			NumberType sinp = TWO<NumberType> * (this->w * this->y - this->z * this->x);
			sinp = sinp > ONE<NumberType> ? ONE<NumberType> : sinp;
			sinp = sinp < -ONE<NumberType> ? -ONE<NumberType> : sinp;

			// yaw
			NumberType siny_cosp = TWO<NumberType> * (this->w * this->z + this->x * this->y);
			NumberType cosy_cosp = ONE<NumberType> - TWO<NumberType> * (this->y * this->y + this->z * this->z);

			return {std::atan2(sinr_cosp, cosr_cosp), std::asin(sinp), std::atan2(siny_cosp, cosy_cosp)};
		}

		Vector3_t<NumberType> toAxis()
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			if (std::abs(this->w) > ONE<NumberType>) {
				auto nrm = this->normalized();

				this->x = nrm.x;
				this->y = nrm.y;
				this->z = nrm.z;
				this->w = nrm.w;
			}

			NumberType den = std::sqrt(ONE<NumberType> - this->w * this->w);
			if (den > static_cast<NumberType>(0.0001F)) return {this->x / den, this->y / den, this->z / den};
			return {1, 0, 0};
		}

		static Vector4_t<NumberType> toQuat(const Vector3_t<NumberType>& in)
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			rawrbox::Vector4_t<NumberType> ret = {};

			NumberType x = in.x * HALFONE<NumberType>;
			NumberType y = in.y * -HALFONE<NumberType>;
			NumberType z = in.z * HALFONE<NumberType>;

			auto sinX = static_cast<NumberType>(std::sin(static_cast<float>(x)));
			auto cosX = static_cast<NumberType>(std::cos(static_cast<float>(x)));
			auto sinY = static_cast<NumberType>(std::sin(static_cast<float>(y)));
			auto cosY = static_cast<NumberType>(std::cos(static_cast<float>(y)));
			auto sinZ = static_cast<NumberType>(std::sin(static_cast<float>(z)));
			auto cosZ = static_cast<NumberType>(std::cos(static_cast<float>(z)));

			ret.w = cosY * cosX * cosZ + sinY * sinX * sinZ;
			ret.x = cosY * sinX * cosZ + sinY * cosX * sinZ;
			ret.y = sinY * cosX * cosZ - cosY * sinX * sinZ;
			ret.z = cosY * cosX * sinZ - sinY * sinX * cosZ;

			return ret;
		}

		static Vector4_t<NumberType> lookRotation(const Vector3_t<NumberType>& lookAt, const Vector3_t<NumberType>& upDirection)
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
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

			auto num8 = (m00 + m11) + m22;
			rawrbox::Vector4_t<NumberType> ret = {};

			if (num8 > ZERO<NumberType>) {
				auto num = std::sqrt(num8 + ONE<NumberType>);

				ret.w = num * HALFONE<NumberType>;
				num = HALFONE<NumberType> / num;

				ret.x = (m12 - m21) * num;
				ret.y = (m20 - m02) * num;
				ret.z = (m01 - m10) * num;
				return ret;
			}

			if ((m00 >= m11) && (m00 >= m22)) {
				auto num7 = std::sqrt(((ONE<NumberType> + m00) - m11) - m22);
				auto num4 = HALFONE<NumberType> / num7;

				ret.x = HALFONE<NumberType> * num7;
				ret.y = (m01 + m10) * num4;
				ret.z = (m02 + m20) * num4;
				ret.w = (m12 - m21) * num4;
				return ret;
			}

			if (m11 > m22) {
				auto num6 = std::sqrt(((ONE<NumberType> + m11) - m00) - m22);
				auto num3 = HALFONE<NumberType> / num6;

				ret.x = (m10 + m01) * num3;
				ret.y = HALFONE<NumberType> * num6;
				ret.z = (m21 + m12) * num3;
				ret.w = (m20 - m02) * num3;
				return ret;
			}

			auto num5 = std::sqrt(((ONE<NumberType> + m22) - m00) - m11);
			auto num2 = HALFONE<NumberType> / num5;

			ret.x = (m20 + m02) * num2;
			ret.y = (m21 + m12) * num2;
			ret.z = HALFONE<NumberType> * num5;
			ret.w = (m01 - m10) * num2;

			return ret;
		}
		// ---------------------

		VecType inverse() {
			auto lengthSq = this->sqrMagnitude();
			if (lengthSq == ZERO<NumberType>) return *this;

			auto i = ONE<NumberType> / lengthSq;
			return VecType(this->xyz() * -i, this->w * i);
		}

		VecType interpolate(const VecType& pEnd, NumberType pFactor) {
			// calc cosine theta
			auto cosom = this->x * pEnd.x + this->y * pEnd.y + this->z * pEnd.z + this->w * pEnd.w;

			// adjust signs (if necessary)
			VecType end = pEnd;
			if (cosom < ZERO<NumberType>) {
				cosom = -cosom;
				end.x = -end.x; // Reverse all signs
				end.y = -end.y;
				end.z = -end.z;
				end.w = -end.w;
			}

			// Calculate coefficients
			// NOLINTBEGIN(clang-analyzer-deadcode.DeadStores)
			NumberType sclp = std::numeric_limits<NumberType>::quiet_NaN();
			NumberType sclq = std::numeric_limits<NumberType>::quiet_NaN();

			if ((ONE<NumberType> - cosom) > static_cast<NumberType>(0.0001)) // 0.0001 -> some epsillon
			{
				// Standard case (slerp)
				NumberType omega = std::numeric_limits<NumberType>::quiet_NaN();
				NumberType sinom = std::numeric_limits<NumberType>::quiet_NaN();

				omega = static_cast<NumberType>(std::acos(cosom)); // extract theta from dot product's cos theta
				sinom = static_cast<NumberType>(std::sin(omega));
				sclp = static_cast<NumberType>(std::sin((ONE<NumberType> - pFactor) * omega) / sinom);
				sclq = static_cast<NumberType>(std::sin(pFactor * omega) / sinom);
			} else {
				// Very close, do linear interp (because it's faster)
				sclp = ONE<NumberType> - pFactor;
				sclq = pFactor;
			}
			// NOLINTEND(clang-analyzer-deadcode.DeadStores)

			VecType pOut;
			pOut.x = sclp * this->x + sclq * end.x;
			pOut.y = sclp * this->y + sclq * end.y;
			pOut.z = sclp * this->z + sclq * end.z;
			pOut.w = sclp * this->w + sclq * end.w;

			return pOut;
		};

		[[nodiscard]] VecType min(const VecType& other) const {
			return {std::min(this->x, other.x), std::min(this->y, other.y), std::min(this->z, other.z), std::min(this->w, other.w)};
		}

		[[nodiscard]] VecType max(const VecType& other) const {
			return {std::max(this->x, other.x), std::max(this->y, other.y), std::max(this->z, other.z), std::min(this->w, other.w)};
		}

		void rotate(NumberType angle, rawrbox::AXIS axis) {
			NumberType halfAngle = angle / TWO<NumberType>;
			auto cosHalfAngle = static_cast<NumberType>(std::cos(halfAngle));
			auto sinHalfAngle = static_cast<NumberType>(std::sin(halfAngle));

			switch (axis) {
				case AXIS::X:
					*this = Vector4_t<NumberType>(cosHalfAngle, sinHalfAngle * ONE<NumberType>, ZERO<NumberType>, ZERO<NumberType>) * (*this);
					return;
				case AXIS::Y:
					*this = Vector4_t<NumberType>(cosHalfAngle, ZERO<NumberType>, sinHalfAngle * ONE<NumberType>, ZERO<NumberType>) * (*this);
					return;
				case AXIS::Z:
					*this = Vector4_t<NumberType>(cosHalfAngle, ZERO<NumberType>, ZERO<NumberType>, sinHalfAngle * ONE<NumberType>) * (*this);
					return;
			}
		}

		// OPERATORS ---
		// numberic typed operators
		VecType operator-(NumberType other) const { return VecType(this->x - other, this->y - other, this->z - other, this->w - other); }
		VecType operator+(NumberType other) const { return VecType(this->x + other, this->y + other, this->z + other, this->w + other); }
		VecType operator*(NumberType other) const { return VecType(this->x * other, this->y * other, this->z * other, this->w * other); }
		VecType operator/(NumberType other) const { return VecType(this->x / other, this->y / other, this->z / other, this->w / other); }

		VecType& operator-=(NumberType other) {
			this->x = this->x - other;
			this->y = this->y - other;
			this->z = this->z - other;
			this->w = this->w - other;
			return *this;
		}

		VecType& operator+=(NumberType other) {
			this->x = this->x + other;
			this->y = this->y + other;
			this->z = this->z + other;
			this->w = this->w + other;
			return *this;
		}

		VecType& operator*=(NumberType other) {
			this->x = this->x * other;
			this->y = this->y * other;
			this->z = this->z * other;
			this->w = this->w * other;
			return *this;
		}

		VecType& operator/=(NumberType other) {
			this->x = this->x / other;
			this->y = this->y / other;
			this->z = this->z / other;
			this->w = this->w / other;
			return *this;
		}

		bool operator==(NumberType other) const { return this->x == other && this->y == other && this->z == other && this->w == other; }
		bool operator!=(NumberType other) const { return !this->operator==(other); }
		bool operator<(NumberType other) const { return this->x < other && this->y < other && this->z < other && this->w < other; }
		bool operator<=(NumberType other) const { return this->x <= other && this->y <= other && this->z <= other && this->w <= other; }
		bool operator>(NumberType other) const { return this->x > other && this->y > other && this->z > other && this->w > other; }
		bool operator>=(NumberType other) const { return this->x >= other && this->y >= other && this->z >= other && this->w >= other; }

		// vector typed operators
		VecType operator-(const VecType& other) const { return VecType(this->x - other.x, this->y - other.y, this->z - other.z, this->w - other.w); }
		VecType operator+(const VecType& other) const { return VecType(this->x + other.x, this->y + other.y, this->z + other.z, this->w + other.w); }
		VecType operator/(const VecType& other) const { return VecType(this->x / other.x, this->y / other.y, this->z / other.z, this->w / other.w); }
		VecType operator*(const VecType& other) const { return VecType(this->w * other.x + this->x * other.w + this->y * other.z - this->z * other.y, this->w * other.y + this->y * other.w + this->z * other.x - this->x * other.z, this->w * other.z + this->z * other.w + this->x * other.y - this->y * other.x, this->w * other.w - this->x * other.x - this->y * other.y - this->z * other.z); }

		VecType& operator-=(const VecType& other) {
			this->x = this->x - other.x;
			this->y = this->y - other.y;
			this->z = this->z - other.z;
			this->w = this->w - other.w;
			return *this;
		}

		VecType& operator+=(const VecType& other) {
			this->x = this->x + other.x;
			this->y = this->y + other.y;
			this->z = this->z + other.z;
			this->w = this->w + other.w;
			return *this;
		}

		VecType& operator*=(const VecType& other) const {
			this->x = this->w * other.x + this->x * other.w + this->y * other.z - this->z * other.y;
			this->y = this->w * other.y + this->y * other.w + this->z * other.x - this->x * other.z;
			this->z = this->w * other.z + this->z * other.w + this->x * other.y - this->y * other.x;
			this->w = this->w * other.w - this->x * other.x - this->y * other.y - this->z * other.z;
			return *this;
		}

		VecType& operator/=(const VecType& other) {
			this->x = this->x / other.x;
			this->y = this->y / other.y;
			this->z = this->z / other.z;
			this->w = this->w / other.w;
			return *this;
		}

		bool operator==(const VecType& other) const { return this->x == other.x && this->y == other.y && this->z == other.z && this->w == other.w; }
		bool operator!=(const VecType& other) const { return !this->operator==(other); }
		bool operator<(const VecType& other) const { return this->x < other.x && this->y < other.y && this->z < other.z && this->w < other.w; }
		bool operator<=(const VecType& other) const { return this->x <= other.x && this->y <= other.y && this->z <= other.z && this->w <= other.w; }
		bool operator>(const VecType& other) const { return this->x > other.x && this->y > other.y && this->z > other.z && this->w > other.w; }
		bool operator>=(const VecType& other) const { return this->x >= other.x && this->y >= other.y && this->z >= other.z && this->w >= other.w; }

		VecType operator-() const { return VecType(-this->x, -this->y, -this->z, -this->w); }

		Vector3_t<NumberType> operator*(const Vector3_t<NumberType>& other) const {
			float num = this->x * TWO<NumberType>;
			float num2 = this->y * TWO<NumberType>;
			float num3 = this->z * TWO<NumberType>;
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
			result.x = (ONE<NumberType> - (num5 + num6)) * other.x + (num7 - num12) * other.y + (num8 + num11) * other.z;
			result.y = (num7 + num12) * other.x + (ONE<NumberType> - (num4 + num6)) * other.y + (num9 - num10) * other.z;
			result.z = (num8 - num11) * other.x + (num9 + num10) * other.y + (ONE<NumberType> - (num4 + num5)) * other.z;
			return result;
		}
		// -------
	};

	using Vector4f = Vector4_t<float>;
	using Vector4i = Vector4_t<int>;
	using Vector4d = Vector4_t<double>;
	using Vector4u = Vector4_t<uint32_t>;
	using Vector4f16 = Vector4_t<uint16_t>;
	using Vector4 = Vector4f;
} // namespace rawrbox
