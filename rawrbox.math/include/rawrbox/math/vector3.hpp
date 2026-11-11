#pragma once

#include <rawrbox/math/pi.hpp>
#include <rawrbox/math/vector2.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <type_traits>

namespace rawrbox {

	enum class AXIS {
		X = 0,
		Y,
		Z
	};

	template <class NumberType>
		requires(std::is_integral_v<NumberType> || std::is_floating_point_v<NumberType>)
	class Vector3_t {
	protected:
		using VecType = Vector3_t<NumberType>;

	public:
		NumberType x = 0;
		NumberType y = 0;
		NumberType z = 0;

		Vector3_t() = default;
		// NOLINTBEGIN(hicpp-explicit-conversions)
		constexpr Vector3_t(NumberType val) : x(val), y(val), z(val) {}
		constexpr Vector3_t(NumberType _x, NumberType _y, NumberType _z) : x(_x), y(_y), z(_z) {}
		constexpr Vector3_t(const std::array<NumberType, 3>& val) : x(val[0]), y(val[1]), z(val[2]) {}
		constexpr Vector3_t(Vector2_t<NumberType> xy, NumberType _z = 0) : x(xy.x), y(xy.y), z(_z) {}
		// NOLINTEND(hicpp-explicit-conversions)

		static VecType zero() { return {}; }
		static VecType one() { return VecType(1, 1, 1); }
		static VecType up() { return VecType(0, 1, 0); }
		static VecType forward() { return VecType(0, 0, 1); }
		static VecType left() { return VecType(1, 0, 0); }

		static VecType nan()
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{ return VecType(std::numeric_limits<NumberType>::quiet_NaN(), std::numeric_limits<NumberType>::quiet_NaN(), std::numeric_limits<NumberType>::quiet_NaN()); }

		[[nodiscard]] bool isNAN() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{ return std::isnan<NumberType>(this->x) && std::isnan<NumberType>(this->y) && std::isnan<NumberType>(this->z); }

		static VecType mad(const VecType& a, const VecType& b, const VecType& c) { return (a * b) + c; }
		static VecType mad(const VecType& a, const NumberType b, const VecType& c) { return (a * b) + c; }

		[[nodiscard]] std::string toString() const { return std::to_string(this->x) + "," + std::to_string(this->y) + "," + std::to_string(this->z); }
		[[nodiscard]] int size() const { return 3; }

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

		[[nodiscard]] std::array<NumberType, 3> data() const { return {this->x, this->y, this->z}; }

		// UTILS ---
		[[nodiscard]] NumberType distance(const VecType& other) const {
			return static_cast<NumberType>(std::sqrt(((this->x - other.x) * (this->x - other.x)) + ((this->y - other.y) * (this->y - other.y)) + ((this->z - other.z) * (this->z - other.z))));
		}

		[[nodiscard]] NumberType length() const {
			return static_cast<NumberType>(std::sqrt(std::pow(this->x, 2) + std::pow(this->y, 2) + std::pow(this->z, 2)));
		}

		[[nodiscard]] NumberType sqrMagnitude() const {
			return static_cast<NumberType>(this->x * this->x + this->y * this->y + this->z * this->z);
		}

		[[nodiscard]] VecType abs() const {
			return {std::abs(this->x), std::abs(this->y), std::abs(this->z)};
		}

		[[nodiscard]] VecType lerp(const VecType& other, float timestep) const
			requires(!std::is_same_v<NumberType, uint16_t>)
		{
			if ((*this) == other) return other;
			VecType ret;

			ret.x = static_cast<NumberType>(static_cast<float>(this->x) + static_cast<float>(other.x - this->x) * timestep);
			ret.y = static_cast<NumberType>(static_cast<float>(this->y) + static_cast<float>(other.y - this->y) * timestep);
			ret.z = static_cast<NumberType>(static_cast<float>(this->z) + static_cast<float>(other.z - this->z) * timestep);

			return ret;
		}

		[[nodiscard]] VecType clamp(NumberType min, NumberType max) const {
			return {
			    std::clamp(this->x, min, max),
			    std::clamp(this->y, min, max),
			    std::clamp(this->z, min, max)};
		}

		[[nodiscard]] VecType clamp(VecType min, VecType max) const {
			return {
			    std::clamp(this->x, min.x, max.x),
			    std::clamp(this->y, min.y, max.y),
			    std::clamp(this->z, min.z, max.z)};
		}

		[[nodiscard]] VecType clampMagnitude(NumberType max) const {
			NumberType mag = this->sqrMagnitude();
			if (mag > max * max) {
				auto m = static_cast<NumberType>(std::sqrt(mag));

				NumberType xx = this->x / m;
				NumberType yy = this->y / m;
				NumberType zz = this->z / m;

				return {xx * max, yy * max, zz * max};
			}

			return *this;
		}

		[[nodiscard]] VecType min(const VecType& other) const {
			return {std::min(this->x, other.x), std::min(this->y, other.y), std::min(this->z, other.z)};
		}

		[[nodiscard]] VecType max(const VecType& other) const {
			return {std::max(this->x, other.x), std::max(this->y, other.y), std::max(this->z, other.z)};
		}

		template <class ReturnType>
			requires(!std::is_same_v<NumberType, ReturnType>)
		Vector3_t<ReturnType> cast() const {
			return {static_cast<ReturnType>(this->x), static_cast<ReturnType>(this->y), static_cast<ReturnType>(this->z)};
		}
		// ------

		// UTILS uint16_t --
		[[nodiscard]] std::array<float, 3> unpack() const
			requires(std::is_same_v<NumberType, uint16_t>)
		{
			float xx = rawrbox::PackUtils::fromFP16(this->x);
			float yy = rawrbox::PackUtils::fromFP16(this->y);
			float zz = rawrbox::PackUtils::fromFP16(this->z);

			return {xx, yy, zz};
		}
		// ----------

		// UTILS - FLOAT ---
		[[nodiscard]] std::array<uint16_t, 3> pack() const
			requires(std::is_same_v<NumberType, float>)
		{
			uint16_t xx = rawrbox::PackUtils::toFP16(this->x);
			uint16_t yy = rawrbox::PackUtils::toFP16(this->y);
			uint16_t zz = rawrbox::PackUtils::toFP16(this->z);

			return {xx, yy, zz};
		}

		// From: https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Vector3.cs#L324
		[[nodiscard]] NumberType angle(const VecType& target) const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			auto denominator = static_cast<NumberType>(std::sqrt(this->sqrMagnitude() * target.sqrMagnitude()));
			if (denominator < 1e-15F)
				return ZERO<NumberType>;

			NumberType dot = std::clamp<NumberType>(this->dot(target) / denominator, -ONE<NumberType>, ONE<NumberType>);
			return std::acos(dot) * (ONE<NumberType> / (rawrbox::pi<NumberType> * static_cast<NumberType>(2.F / 360.F)));
		}

		[[nodiscard]] VecType rotateAroundOrigin(const VecType& axis, float theta) const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			auto cos_theta = static_cast<NumberType>(std::cos(static_cast<float>(theta)));
			auto sin_theta = static_cast<NumberType>(std::sin(static_cast<float>(theta)));

			return ((*this) * cos_theta) + (this->cross(axis) * sin_theta) + (axis * this->dot(axis)) * (ONE<NumberType> - cos_theta);
		}

		[[nodiscard]] VecType normalized() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return (*this) / this->length();
		}

		[[nodiscard]] NumberType dot(const Vector3_t<NumberType>& other) const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return this->x * other.x + this->y * other.y + this->z * other.z;
		}

		[[nodiscard]] VecType floor() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return {std::floor(this->x), std::floor(this->y), std::floor(this->z)};
		}

		[[nodiscard]] VecType round() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return {std::round(this->x), std::round(this->y), std::round(this->z)};
		}

		[[nodiscard]] VecType ceil() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return {std::ceil(this->x), std::ceil(this->y), std::ceil(this->z)};
		}

		[[nodiscard]] VecType cross(const VecType& other) const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return {
			    this->y * other.z - this->z * other.y, this->z * other.x - this->x * other.z, this->x * other.y - this->y * other.x};
		}
		// ----

		// OPERATORS ---
		// numberic typed operators
		VecType operator-(NumberType other) const { return VecType(this->x - other, this->y - other, this->z - other); }
		VecType operator+(NumberType other) const { return VecType(this->x + other, this->y + other, this->z + other); }
		VecType operator*(NumberType other) const { return VecType(this->x * other, this->y * other, this->z * other); }
		VecType operator/(NumberType other) const { return VecType(this->x / other, this->y / other, this->z / other); }

		VecType& operator-=(NumberType other) {
			this->x = this->x - other;
			this->y = this->y - other;
			this->z = this->z - other;
			return *this;
		}
		VecType& operator+=(NumberType other) {
			this->x = this->x + other;
			this->y = this->y + other;
			this->z = this->z + other;
			return *this;
		}
		VecType& operator*=(NumberType other) {
			this->x = this->x * other;
			this->y = this->y * other;
			this->z = this->z * other;
			return *this;
		}
		VecType& operator/=(NumberType other) {
			this->x = this->x / other;
			this->y = this->y / other;
			this->z = this->z / other;
			return *this;
		}

		bool operator==(NumberType other) const { return this->x == other && this->y == other && this->z == other; }
		bool operator!=(NumberType other) const { return !this->operator==(other); }
		bool operator<(NumberType other) const { return this->x < other && this->y < other && this->z < other; }
		bool operator<=(NumberType other) const { return this->x <= other && this->y <= other && this->z <= other; }
		bool operator>(NumberType other) const { return this->x > other && this->y > other && this->z > other; }
		bool operator>=(NumberType other) const { return this->x >= other && this->y >= other && this->z >= other; }

		// vector typed operators
		VecType operator-(const VecType& other) const { return VecType(this->x - other.x, this->y - other.y, this->z - other.z); }
		VecType operator+(const VecType& other) const { return VecType(this->x + other.x, this->y + other.y, this->z + other.z); }
		VecType operator*(const VecType& other) const { return VecType(this->x * other.x, this->y * other.y, this->z * other.z); }
		VecType operator/(const VecType& other) const { return VecType(this->x / other.x, this->y / other.y, this->z / other.z); }

		VecType& operator-=(const VecType& other) {
			this->x = this->x - other.x;
			this->y = this->y - other.y;
			this->z = this->z - other.z;
			return *this;
		}
		VecType& operator+=(const VecType& other) {
			this->x = this->x + other.x;
			this->y = this->y + other.y;
			this->z = this->z + other.z;
			return *this;
		}
		VecType& operator*=(const VecType& other) {
			this->x = this->x * other.x;
			this->y = this->y * other.y;
			this->z = this->z * other.z;
			return *this;
		}
		VecType& operator/=(const VecType& other) {
			this->x = this->x / other.x;
			this->y = this->y / other.y;
			this->z = this->z / other.z;
			return *this;
		}

		bool operator==(const VecType& other) const { return this->x == other.x && this->y == other.y && this->z == other.z; }
		bool operator!=(const VecType& other) const { return !this->operator==(other); }
		bool operator<(const VecType& other) const { return this->x < other.x && this->y < other.y && this->z < other.z; }
		bool operator<=(const VecType& other) const { return this->x <= other.x && this->y <= other.y && this->z <= other.z; }
		bool operator>(const VecType& other) const { return this->x > other.x && this->y > other.y && this->z > other.z; }
		bool operator>=(const VecType& other) const { return this->x >= other.x && this->y >= other.y && this->z >= other.z; }

		VecType operator-() const { return VecType(-this->x, -this->y, -this->z); }
		// -------
	};

	using Vector3f = Vector3_t<float>;
	using Vector3i = Vector3_t<int>;
	using Vector3d = Vector3_t<double>;
	using Vector3u = Vector3_t<uint32_t>;
	using Vector3f16 = Vector3_t<uint16_t>;
	using Vector3 = Vector3f;
} // namespace rawrbox
