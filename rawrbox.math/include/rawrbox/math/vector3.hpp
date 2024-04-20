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
		{ return std::isnan<NumberType>(x) && std::isnan<NumberType>(y) && std::isnan<NumberType>(z); }

		static VecType mad(const VecType& a, const VecType& b, const VecType& c) { return (a * b) + c; }
		static VecType mad(const VecType& a, const NumberType b, const VecType& c) { return (a * b) + c; }

		[[nodiscard]] std::string toString() const { return std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z); }
		[[nodiscard]] int size() const { return 3; }

		[[nodiscard]] Vector3_t<NumberType> yxz() const { return Vector3_t<NumberType>(y, x, z); }
		[[nodiscard]] Vector3_t<NumberType> yzx() const { return Vector3_t<NumberType>(y, z, x); }
		[[nodiscard]] Vector3_t<NumberType> xzy() const { return Vector3_t<NumberType>(x, z, y); }
		[[nodiscard]] Vector3_t<NumberType> zxy() const { return Vector3_t<NumberType>(z, x, y); }
		[[nodiscard]] Vector3_t<NumberType> zyx() const { return Vector3_t<NumberType>(z, y, x); }

		[[nodiscard]] Vector2_t<NumberType> xy() const { return Vector2_t<NumberType>(x, y); }
		[[nodiscard]] Vector2_t<NumberType> yx() const { return Vector2_t<NumberType>(y, x); }
		[[nodiscard]] Vector2_t<NumberType> xz() const { return Vector2_t<NumberType>(x, z); }
		[[nodiscard]] Vector2_t<NumberType> yz() const { return Vector2_t<NumberType>(y, z); }
		[[nodiscard]] Vector2_t<NumberType> zx() const { return Vector2_t<NumberType>(z, x); }
		[[nodiscard]] Vector2_t<NumberType> zy() const { return Vector2_t<NumberType>(z, y); }

		[[nodiscard]] std::array<NumberType, 3> data() const { return {x, y, z}; }

		// UTILS ---
		[[nodiscard]] NumberType distance(const VecType& other) const {
			return static_cast<NumberType>(std::sqrt(((x - other.x) * (x - other.x)) + ((y - other.y) * (y - other.y)) + ((z - other.z) * (z - other.z))));
		}

		[[nodiscard]] NumberType length() const {
			return static_cast<NumberType>(std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2)));
		}

		[[nodiscard]] NumberType sqrMagnitude() const {
			return static_cast<NumberType>(x * x + y * y + z * z);
		}

		[[nodiscard]] VecType abs() const {
			return {std::abs(x), std::abs(y), std::abs(z)};
		}

		[[nodiscard]] VecType lerp(const VecType& other, float timestep) const
			requires(!std::is_same_v<NumberType, uint16_t>)
		{
			if ((*this) == other) return other;
			VecType ret;

			ret.x = static_cast<NumberType>(static_cast<float>(x) + static_cast<float>(other.x - x) * timestep);
			ret.y = static_cast<NumberType>(static_cast<float>(y) + static_cast<float>(other.y - y) * timestep);
			ret.z = static_cast<NumberType>(static_cast<float>(z) + static_cast<float>(other.z - z) * timestep);

			return ret;
		}

		[[nodiscard]] VecType clamp(NumberType min, NumberType max) const {
			return {
			    std::clamp(x, min, max),
			    std::clamp(y, min, max),
			    std::clamp(z, min, max)};
		}

		[[nodiscard]] VecType clamp(VecType min, VecType max) const {
			return {
			    std::clamp(x, min.x, max.x),
			    std::clamp(y, min.y, max.y),
			    std::clamp(z, min.z, max.z)};
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
			return {std::min(x, other.x), std::min(y, other.y), std::min(z, other.z)};
		}

		[[nodiscard]] VecType max(const VecType& other) const {
			return {std::max(x, other.x), std::max(y, other.y), std::max(z, other.z)};
		}

		template <class ReturnType>
			requires(!std::is_same_v<NumberType, ReturnType>)
		Vector3_t<ReturnType> cast() const {
			return {static_cast<ReturnType>(x), static_cast<ReturnType>(y), static_cast<ReturnType>(z)};
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
			auto denominator = static_cast<NumberType>(std::sqrt(sqrMagnitude() * target.sqrMagnitude()));
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
			return (*this) / length();
		}

		[[nodiscard]] NumberType dot(const Vector3_t<NumberType>& other) const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return x * other.x + y * other.y + z * other.z;
		}

		[[nodiscard]] VecType floor() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return {std::floor(x), std::floor(y), std::floor(z)};
		}

		[[nodiscard]] VecType round() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return {std::round(x), std::round(y), std::round(z)};
		}

		[[nodiscard]] VecType ceil() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return {std::ceil(x), std::ceil(y), std::ceil(z)};
		}

		[[nodiscard]] VecType cross(const VecType& other) const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return {
			    y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
		}
		// ----

		// OPERATORS ---
		// numberic typed operators
		VecType operator-(NumberType other) const { return VecType(x - other, y - other, z - other); }
		VecType operator+(NumberType other) const { return VecType(x + other, y + other, z + other); }
		VecType operator*(NumberType other) const { return VecType(x * other, y * other, z * other); }
		VecType operator/(NumberType other) const { return VecType(x / other, y / other, z / other); }

		VecType& operator-=(NumberType other) {
			x = x - other;
			y = y - other;
			z = z - other;
			return *this;
		}
		VecType& operator+=(NumberType other) {
			x = x + other;
			y = y + other;
			z = z + other;
			return *this;
		}
		VecType& operator*=(NumberType other) {
			x = x * other;
			y = y * other;
			z = z * other;
			return *this;
		}
		VecType& operator/=(NumberType other) {
			x = x / other;
			y = y / other;
			z = z / other;
			return *this;
		}

		bool operator==(NumberType other) const { return x == other && y == other && z == other; }
		bool operator!=(NumberType other) const { return !operator==(other); }
		bool operator<(NumberType other) const { return x < other && y < other && z < other; }
		bool operator<=(NumberType other) const { return x <= other && y <= other && z <= other; }
		bool operator>(NumberType other) const { return x > other && y > other && z > other; }
		bool operator>=(NumberType other) const { return x >= other && y >= other && z >= other; }

		// vector typed operators
		VecType operator-(const VecType& other) const { return VecType(x - other.x, y - other.y, z - other.z); }
		VecType operator+(const VecType& other) const { return VecType(x + other.x, y + other.y, z + other.z); }
		VecType operator*(const VecType& other) const { return VecType(x * other.x, y * other.y, z * other.z); }
		VecType operator/(const VecType& other) const { return VecType(x / other.x, y / other.y, z / other.z); }

		VecType& operator-=(const VecType& other) {
			x = x - other.x;
			y = y - other.y;
			z = z - other.z;
			return *this;
		}
		VecType& operator+=(const VecType& other) {
			x = x + other.x;
			y = y + other.y;
			z = z + other.z;
			return *this;
		}
		VecType& operator*=(const VecType& other) {
			x = x * other.x;
			y = y * other.y;
			z = z * other.z;
			return *this;
		}
		VecType& operator/=(const VecType& other) {
			x = x / other.x;
			y = y / other.y;
			z = z / other.z;
			return *this;
		}

		bool operator==(const VecType& other) const { return x == other.x && y == other.y && z == other.z; }
		bool operator!=(const VecType& other) const { return !operator==(other); }
		bool operator<(const VecType& other) const { return x < other.x && y < other.y && z < other.z; }
		bool operator<=(const VecType& other) const { return x <= other.x && y <= other.y && z <= other.z; }
		bool operator>(const VecType& other) const { return x > other.x && y > other.y && z > other.z; }
		bool operator>=(const VecType& other) const { return x >= other.x && y >= other.y && z >= other.z; }

		VecType operator-() const { return VecType(-x, -y, -z); }
		// -------
	};

	using Vector3f = Vector3_t<float>;
	using Vector3i = Vector3_t<int>;
	using Vector3d = Vector3_t<double>;
	using Vector3u = Vector3_t<uint32_t>;
	using Vector3f16 = Vector3_t<uint16_t>;
	using Vector3 = Vector3f;
} // namespace rawrbox
