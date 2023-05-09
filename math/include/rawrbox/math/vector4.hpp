#pragma once

#include <rawrbox/math/pi.hpp>
#include <rawrbox/math/vector2.hpp>

#pragma once

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
		Vector4_t(NumberType _x, NumberType _y, NumberType _z, NumberType _w) : x(_x), y(_y), z(_z), w(_w) {}

		static VecType zero() { return VecType(); }
		static VecType one() { return VecType(1, 1, 1, 1); }

		NumberType length() const {
			return static_cast<NumberType>(std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2) + std::pow(w, 2)));
		}

		VecType normalized() const {
			return (*this) / length();
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
		VecType operator*(const VecType& other) const { return VecType(x * other.x, y * other.y, z * other.z, w * other.w); }
		VecType operator/(const VecType& other) const { return VecType(x / other.x, y / other.y, z / other.z, w / other.w); }

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
		VecType& operator*=(const VecType& other) {
			x = x * other.x;
			y = y * other.y;
			z = z * other.z;
			w = w * other.w;
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
		// -------
	};

	using Vector4f = Vector4_t<float>;
	using Vector4i = Vector4_t<int>;
	using Vector4 = Vector4f;
} // namespace rawrbox
