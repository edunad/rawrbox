#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>

#include <algorithm>

namespace rawrBox {
	template <class NumberType>
	class Quaternion_t {
	private:
		using QuaternionType = Quaternion_t<NumberType>;

	public:
		NumberType w = 0, x = 0, y = 0, z = 0;

		Quaternion_t() = default;
		Quaternion_t(NumberType _w, NumberType _x, NumberType _y, NumberType _z) : w(_w), x(_x), y(_y), z(_z) {}

		QuaternionType operator-(const QuaternionType& other) const {
			return QuaternionType(w - other.w, x - other.x, y - other.y, z - other.z);
		}

		QuaternionType operator+(const QuaternionType& other) const {
			return QuaternionType(w + other.w, x + other.x, y + other.y, z + other.z);
		}

		QuaternionType operator*(const QuaternionType& other) const {
			return QuaternionType(w * other.w, x * other.x, y * other.y, z * other.z);
		}

		QuaternionType operator/(const QuaternionType& other) const {
			return QuaternionType(w / other.w, x / other.x, y / other.y, z / other.z);
		}

		QuaternionType operator-(NumberType other) const {
			return QuaternionType(w - other, x - other, y - other, z - other);
		}

		QuaternionType operator+(NumberType other) const {
			return QuaternionType(w + other, x + other, y + other, z + other);
		}

		QuaternionType operator*(NumberType other) const {
			return QuaternionType(w * other, x * other, y * other, z * other);
		}

		QuaternionType operator/(NumberType other) const {
			return QuaternionType(w / other, x / other, y / other, z / other);
		}

		NumberType length() const {
			return static_cast<NumberType>(std::sqrt(std::pow(w, 2) + std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2)));
		}

		QuaternionType normalized() const {
			return (*this) / length();
		}

		QuaternionType lerp(const QuaternionType& other, NumberType timestep) const {
			if ((*this) == other) return other;
			QuaternionType ret;

			float dot = w * other.w + x * other.x + y * other.y + z * other.z;
			float blend = 1.F - timestep;

			if (dot < 0.F) {
				ret.w = blend * w + blend * -other.w;
				ret.x = blend * x + blend * -other.x;
				ret.y = blend * y + blend * -other.y;
				ret.z = blend * z + blend * -other.z;
			} else {
				ret.w = blend * w + blend * other.w;
				ret.x = blend * x + blend * other.x;
				ret.y = blend * y + blend * other.y;
				ret.z = blend * z + blend * other.z;
			}

			return ret.normalized();
		}

		QuaternionType& operator-=(const QuaternionType& other) {
			w -= other.w;
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		QuaternionType& operator+=(const QuaternionType& other) {
			w += other.w;
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		QuaternionType& operator*=(const QuaternionType& other) {
			w *= other.w;
			x *= other.x;
			y *= other.y;
			z *= other.z;
			return *this;
		}

		QuaternionType& operator/=(const QuaternionType& other) {
			w /= other.w;
			x /= other.x;
			y /= other.y;
			z /= other.z;
			return *this;
		}

		QuaternionType& operator-=(NumberType other) {
			w -= other;
			x -= other;
			y -= other;
			z -= other;
			return *this;
		}

		QuaternionType& operator+=(NumberType other) {
			w += other;
			x += other;
			y += other;
			z += other;
			return *this;
		}

		QuaternionType& operator*=(NumberType other) {
			w *= other;
			x *= other;
			y *= other;
			z *= other;
			return *this;
		}

		QuaternionType& operator/=(NumberType other) {
			w /= other;
			x /= other;
			y /= other;
			z /= other;
			return *this;
		}

		QuaternionType operator-() const {
			return QuaternionType(-w, -x, -y, -z);
		}

		bool operator==(const QuaternionType& other) const {
			return w == other.w && x == other.x && y == other.y && z == other.z;
		}

		bool operator!=(const QuaternionType& other) const {
			return !operator==(other);
		}
	};

	using Quaterniond = Quaternion_t<double>;
	using Quaternionf = Quaternion_t<float>;
	using Quaternioni = Quaternion_t<int>;
	using Quaternion = Quaternionf;

} // namespace rawrBox
