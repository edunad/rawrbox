#pragma once

#include <rawrbox/math/pi.hpp>
#include <rawrbox/math/vector2.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <type_traits>

namespace rawrbox {
	template <class NumberType>
	class Vector3_t {
	protected:
		using VecType = Vector3_t<NumberType>;

	public:
		NumberType x = 0, y = 0, z = 0;

		Vector3_t() = default;

		explicit Vector3_t(NumberType val) : x(val), y(val), z(val) {}
		explicit Vector3_t(const std::array<NumberType, 3>& val) : x(val[0]), y(val[1]), z(val[2]) {}
		explicit Vector3_t(Vector2_t<NumberType> xy, NumberType _z = 0) : x(xy.x), y(xy.y), z(_z) {}
		Vector3_t(NumberType _x, NumberType _y, NumberType _z = 0) : x(_x), y(_y), z(_z) {}

		static VecType zero() { return VecType(); }
		static VecType one() { return VecType(1, 1, 1); }

		[[nodiscard]] Vector3_t<NumberType> xyz() const { return Vector3_t<NumberType>(x, y, z); }
		[[nodiscard]] Vector3_t<NumberType> yxz() const { return Vector3_t<NumberType>(y, x, z); }
		[[nodiscard]] Vector3_t<NumberType> yzx() const { return Vector3_t<NumberType>(y, z, x); }
		[[nodiscard]] Vector3_t<NumberType> xzy() const { return Vector3_t<NumberType>(x, z, y); }
		[[nodiscard]] Vector3_t<NumberType> zxy() const { return Vector3_t<NumberType>(z, x, y); }
		[[nodiscard]] Vector3_t<NumberType> zyx() const { return Vector3_t<NumberType>(z, y, x); }

		[[nodiscard]] Vector2_t<NumberType> xy() const { return Vector2_t<NumberType>(x, y); }
		[[nodiscard]] Vector2_t<NumberType> yx() const { return Vector2_t<NumberType>(y, x); }

		[[nodiscard]] const std::array<NumberType, 3> data() const { return {x, y, z}; }

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

		// From: https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Vector3.cs#L324
		[[nodiscard]] NumberType angle(const VecType& target) const {
			float denominator = std::sqrt(sqrMagnitude() * target.sqrMagnitude());
			if (denominator < 1e-15F)
				return 0.F;

			float dot = std::clamp(this->dot(target) / denominator, -1.F, 1.F);
			return std::acos(dot) * (1.F / (rawrbox::pi<float> * 2.F / 360.F));
		}

		[[nodiscard]] VecType lerp(const VecType& other, float timestep) const {
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

		template <class ReturnType>
		Vector3_t<ReturnType> cast() const {
			return {static_cast<ReturnType>(x), static_cast<ReturnType>(y), static_cast<ReturnType>(z)};
		}

		[[nodiscard]] std::array<NumberType, 3> toArray() const { return {x, y, z}; }
		// ------

		// UTILS - FLOAT ---
		[[nodiscard]] VecType normalized() const
			requires(std::is_same<NumberType, float>::value)
		{
			return (*this) / length();
		}

		[[nodiscard]] float dot(const Vector3_t<float>& other) const
			requires(std::is_same<NumberType, float>::value)
		{
			return x * other.x + y * other.y + z * other.z;
		}

		[[nodiscard]] VecType floor() const
			requires(std::is_same<NumberType, float>::value)
		{
			return {std::floor(x), std::floor(y), std::floor(z)};
		}

		[[nodiscard]] VecType round() const
			requires(std::is_same<NumberType, float>::value)
		{
			return {std::round(x), std::round(y), std::round(z)};
		}

		[[nodiscard]] VecType ceil() const
			requires(std::is_same<NumberType, float>::value)
		{
			return {std::ceil(x), std::ceil(y), std::ceil(z)};
		}

		[[nodiscard]] VecType cross(const VecType& other) const
			requires(std::is_same<NumberType, float>::value)
		{
			VecType retVal;
			retVal.x = y * other.z - z * other.y;
			retVal.y = z * other.x - x * other.z;
			retVal.z = x * other.y - y * other.x;

			return retVal;
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
	using Vector3 = Vector3f;
} // namespace rawrbox
