#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <string>
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

		// NOLINTBEGIN(hicpp-explicit-conversions)
		Vector2_t(std::array<NumberType, 2> val) : x(val[0]), y(val[1]) {}
		// NOLINTEND(hicpp-explicit-conversions)

		static VecType zero() { return VecType(); }
		static VecType one() { return VecType(1, 1); }
		static VecType nan() { return VecType(std::numeric_limits<NumberType>::quiet_NaN(), std::numeric_limits<NumberType>::quiet_NaN()); }

		[[nodiscard]] std::string toString() const { return std::to_string(x) + "," + std::to_string(y); }
		[[nodiscard]] int size() const { return 2; }

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

		[[nodiscard]] VecType min(const VecType& other) const
			requires(std::is_same_v<NumberType, float>)
		{
			return {std::min(x, other.x), std::min(y, other.y)};
		}

		[[nodiscard]] VecType max(const VecType& other) const
			requires(std::is_same_v<NumberType, float>)
		{
			return {std::max(x, other.x), std::max(y, other.y)};
		}

		[[nodiscard]] NumberType atan2() const {
			return std::atan2(y, x);
		}

		static VecType sinCos(float radians) {
			return VecType(std::sin(radians), std::cos(radians));
		}

		static VecType cosSin(float radians) {
			return VecType(std::cos(radians), std::sin(radians));
		}

		static VecType intersects(const VecType& a1, const VecType& a2, const VecType& b1, const VecType& b2) {
			float ua = (b2.x - b1.x) * (a1.y - b1.y) - (b2.y - b1.y) * (a1.x - b1.x);
			float ub = (a2.x - a1.x) * (a1.y - b1.y) - (a2.y - a1.y) * (a1.x - b1.x);
			float denominator = (b2.y - b1.y) * (a2.x - a1.x) - (b2.x - b1.x) * (a2.y - a1.y);

			if (std::abs(denominator) > 0.00001F) {
				ua /= denominator;
				ub /= denominator;

				if (ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1)
					return {
					    a1.x + ua * (a2.x - a1.x),
					    a1.y + ua * (a2.y - a1.y)};
			}

			return {
			    std::numeric_limits<float>::quiet_NaN(),
			    std::numeric_limits<float>::quiet_NaN()};
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
			requires(std::is_same_v<NumberType, float>)
		{
			return x * other.x + y * other.y;
		}

		[[nodiscard]] Vector2_t<float> normalized() const
			requires(std::is_same_v<NumberType, float>)
		{
			float l = length();
			return l == 0 ? Vector2_t<float>() : (*this) / l;
		}

		[[nodiscard]] float cross(const Vector2_t<float>& other) const
			requires(std::is_same_v<NumberType, float>)
		{
			return x * other.y - y * other.x;
		}

		[[nodiscard]] Vector2_t<float> floor() const
			requires(std::is_same_v<NumberType, float>)
		{
			return {std::floor(x), std::floor(y)};
		}

		[[nodiscard]] Vector2_t<float> round() const
			requires(std::is_same_v<NumberType, float>)
		{
			return {std::round(x), std::round(y)};
		}

		[[nodiscard]] Vector2_t<float> ceil() const
			requires(std::is_same_v<NumberType, float>)
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
