#pragma once

#include <rawrbox/math/pi.hpp>
#include <rawrbox/math/utils/pack.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <string>
#include <type_traits>

namespace rawrbox {
	template <class NumberType>
		requires(std::is_integral_v<NumberType> || std::is_floating_point_v<NumberType>)
	class Vector2_t {
	protected:
		using VecType = Vector2_t<NumberType>;

	public:
		NumberType x = 0, y = 0;

		Vector2_t() = default;
		constexpr explicit Vector2_t(NumberType val) : x(val), y(val) {}
		constexpr Vector2_t(NumberType _x, NumberType _y) : x(_x), y(_y) {}

		// NOLINTBEGIN(hicpp-explicit-conversions)
		constexpr Vector2_t(std::array<NumberType, 2> val) : x(val[0]), y(val[1]) {}
		// NOLINTEND(hicpp-explicit-conversions)

		static VecType zero() { return VecType(); }
		static VecType one() { return VecType(1, 1); }

		static VecType nan()
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{ return VecType(std::numeric_limits<NumberType>::quiet_NaN(), std::numeric_limits<NumberType>::quiet_NaN()); }

		[[nodiscard]] bool isNAN() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{ return std::isnan<NumberType>(x) && std::isnan<NumberType>(y); }

		[[nodiscard]] std::string toString() const { return std::to_string(x) + "," + std::to_string(y); }
		[[nodiscard]] int size() const { return 2; }

		[[nodiscard]] Vector2_t<NumberType> yx() const { return Vector2_t<NumberType>(y, x); }
		[[nodiscard]] std::array<NumberType, 2> data() const { return {x, y}; }

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

		[[nodiscard]] VecType lerp(const VecType& other, float timestep) const
			requires(!std::is_same_v<NumberType, uint16_t>)
		{
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

		[[nodiscard]] NumberType sqrMagnitude() const {
			return static_cast<NumberType>(std::pow(x, 2) + std::pow(y, 2));
		}

		[[nodiscard]] VecType clampMagnitude(NumberType max) const {
			NumberType mag = this->sqrMagnitude();
			if (mag > max * max) {
				auto m = static_cast<NumberType>(std::sqrt(mag));

				NumberType _x = this->x / m;
				NumberType _y = this->y / m;

				return {_x * max, _y * max};
			}

			return *this;
		}

		[[nodiscard]] VecType min(const VecType& other) const {
			return {std::min(x, other.x), std::min(y, other.y)};
		}

		[[nodiscard]] VecType max(const VecType& other) const {
			return {std::max(x, other.x), std::max(y, other.y)};
		}

		[[nodiscard]] NumberType atan2() const {
			return static_cast<NumberType>(std::atan2(y, x));
		}

		// UTILS uint16_t --
		[[nodiscard]] std::array<float, 2> unpack() const
			requires(std::is_same_v<NumberType, uint16_t>)
		{
			float xx = rawrbox::PackUtils::fromFP16(this->x);
			float yy = rawrbox::PackUtils::fromFP16(this->y);

			return {xx, yy};
		}
		// ----------

		[[nodiscard]] std::array<uint16_t, 2> pack() const
			requires(std::is_same_v<NumberType, float>)
		{
			uint16_t xx = rawrbox::PackUtils::toFP16(this->x);
			uint16_t yy = rawrbox::PackUtils::toFP16(this->y);

			return {xx, yy};
		}

		static VecType sinCos(NumberType radians) {
			return {std::sin(radians), std::cos(radians)};
		}

		static VecType cosSin(NumberType radians)
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return {std::cos(radians), std::sin(radians)};
		}

		static VecType intersects(const VecType& a1, const VecType& a2, const VecType& b1, const VecType& b2) {
			auto ua = (b2.x - b1.x) * (a1.y - b1.y) - (b2.y - b1.y) * (a1.x - b1.x);
			auto ub = (a2.x - a1.x) * (a1.y - b1.y) - (a2.y - a1.y) * (a1.x - b1.x);
			auto denominator = (b2.y - b1.y) * (a2.x - a1.x) - (b2.x - b1.x) * (a2.y - a1.y);

			if (std::abs(denominator) > static_cast<NumberType>(0.00001F)) {
				ua /= denominator;
				ub /= denominator;

				if (ua >= rawrbox::ZERO<NumberType> && ua <= rawrbox::ONE<NumberType> && ub >= rawrbox::ZERO<NumberType> && ub <= rawrbox::ONE<NumberType>)
					return {
					    static_cast<NumberType>(a1.x + ua * (a2.x - a1.x)),
					    static_cast<NumberType>(a1.y + ua * (a2.y - a1.y))};
			}

			return {
			    std::numeric_limits<NumberType>::quiet_NaN(),
			    std::numeric_limits<NumberType>::quiet_NaN()};
		}

		[[nodiscard]] VecType rotateAroundOrigin(NumberType rads, const VecType& origin) const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			if (rads == rawrbox::ZERO<NumberType>) return *this;

			VecType u = *this - origin;
			if (u == VecType()) return *this;

			u = cosSin(u.atan2() + rads) * static_cast<NumberType>(u.length());
			u.x += origin.x;
			u.y += origin.y;

			return u;
		}

		template <class ReturnType>
			requires(!std::is_same_v<NumberType, ReturnType>)
		Vector2_t<ReturnType> cast() const {
			return {static_cast<ReturnType>(x), static_cast<ReturnType>(y)};
		}
		// ------

		[[nodiscard]] NumberType dot(const VecType& other) const {
			return x * other.x + y * other.y;
		}

		// UTILS - FLOAT ---
		[[nodiscard]] VecType normalized() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			auto l = length();
			return l == rawrbox::ZERO<NumberType> ? VecType() : (*this) / l;
		}

		[[nodiscard]] NumberType cross(const VecType& other) const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return x * other.y - y * other.x;
		}

		[[nodiscard]] Vector2_t<NumberType> floor() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return {std::floor(x), std::floor(y)};
		}

		[[nodiscard]] Vector2_t<NumberType> round() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
		{
			return {std::round(x), std::round(y)};
		}

		[[nodiscard]] Vector2_t<NumberType> ceil() const
			requires(std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>)
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
	using Vector2u = Vector2_t<uint32_t>;
	using Vector2f16 = Vector2_t<uint16_t>;
	using Vector2 = Vector2f;
} // namespace rawrbox
