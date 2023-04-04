#pragma once

#include <rawrbox/math/vector2.h>
#include <rawrbox/math/vector3.h>

#include <algorithm>

namespace mainframe {
	namespace math {
		template <class NumberType>
		class Vector4_t {
		private:
			using VecType = Vector4_t<NumberType>;

		public:
			NumberType x = 0, y = 0, z = 0, w = 0;

			Vector4_t() = default;
			Vector4_t(Vector2_t<NumberType> first, Vector2_t<NumberType> second = 0) : x(first.x), y(first.y), z(second.x), w(second.y) {}
			Vector4_t(Vector3_t<NumberType> vec, NumberType _w = 0) : x(vec.x), y(vec.y), z(vec.z), w(_w) {}
			Vector4_t(NumberType _x, NumberType _y = 0, NumberType _z = 0, NumberType _w = 0) : x(_x), y(_y), z(_z), w(_w) {}

			VecType operator- (const VecType& other) const {
				return VecType(x - other.x, y - other.y, z - other.z, w - other.w);
			}

			VecType operator+ (const VecType& other) const {
				return VecType(x + other.x, y + other.y, z + other.z, w + other.w);
			}

			VecType operator* (const VecType& other) const {
				return VecType(x * other.x, y * other.y, z * other.z, w * other.w);
			}

			VecType operator/ (const VecType& other) const {
				return VecType(x / other.x, y / other.y, z / other.z, w / other.w);
			}

			VecType operator- (NumberType other) const {
				return VecType(x - other, y - other, z - other, w - other);
			}

			VecType operator+ (NumberType other) const {
				return VecType(x + other, y + other, z + other, w + other);
			}

			VecType operator* (NumberType other) const {
				return VecType(x * other, y * other, z * other, w * other);
			}

			VecType operator/ (NumberType other) const {
				return VecType(x / other, y / other, z / other, w / other);
			}

			NumberType length() const {
				return static_cast<NumberType>(std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2) + std::pow(w, 2)));
			}

			VecType normalized() const {
				return (*this) / length();
			}

			VecType lerp(const VecType& other, NumberType timestep) const {
				if((*this) == other) return other;
				VecType ret;

				ret.x = static_cast<NumberType>(static_cast<float>(x) + static_cast<float>(other.x - x) * timestep);
				ret.y = static_cast<NumberType>(static_cast<float>(y) + static_cast<float>(other.y - y) * timestep);
				ret.z = static_cast<NumberType>(static_cast<float>(z) + static_cast<float>(other.z - z) * timestep);
				ret.w = static_cast<NumberType>(static_cast<float>(w) + static_cast<float>(other.w - w) * timestep);

				return ret;
			}

			VecType& operator-= (const VecType& other) {
				x -= other.x;
				y -= other.y;
				z -= other.z;
				w -= other.w;
				return *this;
			}

			VecType& operator+= (const VecType& other) {
				x += other.x;
				y += other.y;
				z += other.z;
				w += other.w;
				return *this;
			}

			VecType& operator*= (const VecType& other) {
				x *= other.x;
				y *= other.y;
				z *= other.z;
				w *= other.w;
				return *this;
			}

			VecType& operator/= (const VecType& other) {
				x /= other.x;
				y /= other.y;
				z /= other.z;
				w /= other.w;
				return *this;
			}

			VecType& operator-= (NumberType other) {
				x -= other;
				y -= other;
				z -= other;
				w -= other;
				return *this;
			}

			VecType& operator+= (NumberType other) {
				x += other;
				y += other;
				z += other;
				w += other;
				return *this;
			}

			VecType& operator*= (NumberType other) {
				x *= other;
				y *= other;
				z *= other;
				w *= other;
				return *this;
			}

			VecType& operator/= (NumberType other) {
				x /= other;
				y /= other;
				z /= other;
				w /= other;
				return *this;
			}

			VecType operator-() const {
				return VecType(-x, -y, -z, -w);
			}

			bool operator== (const VecType& other) const {
				return x == other.x && y == other.y && z == other.z && w == other.w;
			}

			bool operator!= (const VecType& other) const {
				return !operator==(other);
			}
		};

		using Vector4d = Vector4_t<double>;
		using Vector4f = Vector4_t<float>;
		using Vector4i = Vector4_t<int>;
		using Vector4 = Vector4f;
	}
}
