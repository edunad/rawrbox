#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <type_traits>

namespace rawrbox {
	template <class NumberType>
	class Vector2_t {
	protected:
		using VecType = Vector2_t<NumberType>;

	public:
		NumberType x = 0, y = 0;

		Vector2_t() = default;
		explicit Vector2_t(NumberType val) : x(val), y(val) {}
		Vector2_t(NumberType _x, NumberType _y) : x(_x), y(_y) {}

		static VecType zero() { return VecType(); }
		static VecType one() { return VecType(1, 1); }

		[[nodiscard]] Vector2_t<NumberType> yx() const { return Vector2_t<NumberType>(y, x); }
		[[nodiscard]] const std::array<NumberType, 2> data() const { return {x, y}; }

		// UTILS ---
		[[nodiscard]] NumberType distance(const VecType& other) const {
			return static_cast<NumberType>(std::sqrt(((x - other.x) * (x - other.x)) + ((y - other.y) * (y - other.y))));
		}

		[[nodiscard]] NumberType length() const {
			return static_cast<NumberType>(std::sqrt(std::pow(x, 2) + std::pow(y, 2)));
		}

		[[nodiscard]] NumberType angle(const VecType& target) const {
			return static_cast<NumberType>(std::atan2(target.x - x, target.y - y));
		}

		[[nodiscard]] VecType abs() const {
			return {std::abs(x), std::abs(y)};
		}

		[[nodiscard]] VecType lerp(const VecType& other, float timestep) const {
			VecType ret;

			ret.x = static_cast<NumberType>(static_cast<float>(x) + static_cast<float>(other.x - x) * timestep);
			ret.y = static_cast<NumberType>(static_cast<float>(y) + static_cast<float>(other.y - y) * timestep);

			return ret;
		}

		[[nodiscard]] VecType clamp(NumberType min, NumberType max) const {
			return {
			    std::clamp(x, min, max),
			    std::clamp(y, min, max)};
		}

		[[nodiscard]] VecType clamp(VecType min, VecType max) const {
			return {
			    std::clamp(x, min.x, max.x),
			    std::clamp(y, min.y, max.y)};
		}

		[[nodiscard]] NumberType atan2() const {
			return std::atan2(y, x);
		}

		static VecType sinCos(float radians) {
			return VecType(std::sin(radians), std::cos(radians));
		}

		[[nodiscard]] VecType rotateAroundOrigin(NumberType rads, const VecType& origin) const {
			if (rads == 0) return *this;

			VecType u = *this - origin;
			if (u == VecType()) return *this;

			u = cosSin(u.atan2() + rads) * static_cast<NumberType>(u.length());
			u.x += origin.x;
			u.y += origin.y;

			return u;
		}

		template <class ReturnType>
		Vector2_t<ReturnType> cast() const {
			return {static_cast<ReturnType>(x), static_cast<ReturnType>(y)};
		}
		// ------

		// UTILS - FLOAT ---
		[[nodiscard]] float dot(const Vector2_t<float>& other) const
			requires(std::is_same<NumberType, float>::value)
		{
			return x * other.x + y * other.y;
		}

		[[nodiscard]] Vector2_t<float> normalized() const
			requires(std::is_same<NumberType, float>::value)
		{
			float l = length();
			return l == 0 ? Vector2_t<float>() : (*this) / l;
		}

		[[nodiscard]] float cross(const Vector2_t<float>& other) const
			requires(std::is_same<NumberType, float>::value)
		{
			return x * other.y - y * other.x;
		}

		[[nodiscard]] Vector2_t<float> floor() const
			requires(std::is_same<NumberType, float>::value)
		{
			return {std::floor(x), std::floor(y)};
		}

		[[nodiscard]] Vector2_t<float> round() const
			requires(std::is_same<NumberType, float>::value)
		{
			return {std::round(x), std::round(y)};
		}

		[[nodiscard]] Vector2_t<float> ceil() const
			requires(std::is_same<NumberType, float>::value)
		{
			return {std::ceil(x), std::ceil(y)};
		}

		[[nodiscard]] std::array<NumberType, 2> toArray() const { return {x, y}; }
		// ----

		// OPERATORS ---
		// numberic typed operators
		VecType operator-(NumberType other) const { return VecType(x - other, y - other); }
		VecType operator+(NumberType other) const { return VecType(x + other, y + other); }
		VecType operator*(NumberType other) const { return VecType(x * other, y * other); }
		VecType operator/(NumberType other) const { return VecType(x / other, y / other); }

		VecType& operator-=(NumberType other) {
			x = x - other;
			y = y - other;
			return *this;
		}
		VecType& operator+=(NumberType other) {
			x = x + other;
			y = y + other;
			return *this;
		}
		VecType& operator*=(NumberType other) {
			x = x * other;
			y = y * other;
			return *this;
		}
		VecType& operator/=(NumberType other) {
			x = x / other;
			y = y / other;
			return *this;
		}

		bool operator==(NumberType other) const { return x == other && y == other; }
		bool operator!=(NumberType other) const { return !operator==(other); }
		bool operator<(NumberType other) const { return x < other && y < other; }
		bool operator<=(NumberType other) const { return x <= other && y <= other; }
		bool operator>(NumberType other) const { return x > other && y > other; }
		bool operator>=(NumberType other) const { return x >= other && y >= other; }

		// vector typed operators
		VecType operator-(const VecType& other) const { return VecType(x - other.x, y - other.y); }
		VecType operator+(const VecType& other) const { return VecType(x + other.x, y + other.y); }
		VecType operator*(const VecType& other) const { return VecType(x * other.x, y * other.y); }
		VecType operator/(const VecType& other) const { return VecType(x / other.x, y / other.y); }

		VecType& operator-=(const VecType& other) {
			x = x - other.x;
			y = y - other.y;
			return *this;
		}
		VecType& operator+=(const VecType& other) {
			x = x + other.x;
			y = y + other.y;
			return *this;
		}
		VecType& operator*=(const VecType& other) {
			x = x * other.x;
			y = y * other.y;
			return *this;
		}
		VecType& operator/=(const VecType& other) {
			x = x / other.x;
			y = y / other.y;
			return *this;
		}

		bool operator==(const VecType& other) const { return x == other.x && y == other.y; }
		bool operator!=(const VecType& other) const { return !operator==(other); }
		bool operator<(const VecType& other) const { return x < other.x && y < other.y; }
		bool operator<=(const VecType& other) const { return x <= other.x && y <= other.y; }
		bool operator>(const VecType& other) const { return x > other.x && y > other.y; }
		bool operator>=(const VecType& other) const { return x >= other.x && y >= other.y; }

		VecType operator-() const { return VecType(-x, -y); }
		// -------
	};

	using Vector2f = Vector2_t<float>;
	using Vector2i = Vector2_t<int>;
	using Vector2 = Vector2f;
} // namespace rawrbox
