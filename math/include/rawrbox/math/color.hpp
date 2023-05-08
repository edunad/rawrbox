#pragma once

#include <rawrbox/utils/pack.hpp>

#include <algorithm>
#include <array>
#include <complex>
#include <vector>

namespace rawrbox {
	template <class NumberType>
	class Color_t {
	private:
		using ColorType = Color_t<NumberType>;

	public:
		NumberType r = 0, g = 0, b = 0, a = 1;

		Color_t() = default;
		Color_t(NumberType _r, NumberType _g, NumberType _b, NumberType _a) : r(_r), g(_g), b(_b), a(_a) {}

		static Color_t<NumberType> RGBAHex(uint32_t x) {
			if constexpr (std::is_same<NumberType, float>::value) {
				return {
				    ((x >> 24) & 0xFF) / 255.0F,
				    ((x >> 16) & 0xFF) / 255.0F,
				    ((x >> 8) & 0xFF) / 255.0F,
				    ((x)&0xFF) / 255.0F};
			} else {
				return {
				    ((x >> 24) & 0xFF),
				    ((x >> 16) & 0xFF),
				    ((x >> 8) & 0xFF),
				    ((x)&0xFF)};
			}
		}

		static Color_t<NumberType> RGBHex(uint32_t x) {
			if constexpr (std::is_same<NumberType, float>::value) {
				return {
				    ((x >> 16) & 0xFF) / 255.0F,
				    ((x >> 8) & 0xFF) / 255.0F,
				    ((x)&0xFF) / 255.0F,
				    1.F};
			} else {
				return {
				    static_cast<int>(((x >> 16) & 0xFF)),
				    static_cast<int>(((x >> 8) & 0xFF)),
				    static_cast<int>(((x)&0xFF)),
				    255};
			}
		}

		template <class ReturnType>
		[[nodiscard]] Color_t<ReturnType> cast() const {
			if constexpr (std::is_same<NumberType, int>::value) {
				return Color_t<float>(
				    static_cast<ReturnType>(r) / 255.0F,
				    static_cast<ReturnType>(g) / 255.0F,
				    static_cast<ReturnType>(b) / 255.0F,
				    static_cast<ReturnType>(a) / 255.0F);
			} else {
				return Color_t<int>(
				    static_cast<ReturnType>(r * 255.0F),
				    static_cast<ReturnType>(g * 255.0F),
				    static_cast<ReturnType>(b * 255.0F),
				    static_cast<ReturnType>(a * 255.0F));
			}
		}

		static ColorType debug(int index) {
			std::array cols{
			    ColorType(255, 82, 82, 255),   // rgb(255, 82, 82)
			    ColorType(51, 217, 178, 255),  // rgb(51, 217, 178)
			    ColorType(52, 172, 224, 255),  // rgb(52, 172, 224)
			    ColorType(112, 111, 211, 255), // rgb(112, 111, 211)
			    ColorType(179, 55, 113, 255),  // rgb(179, 55, 113)
			    ColorType(255, 0, 255, 255),   // rgb(252, 0, 255)
			    ColorType(241, 196, 15, 255),  // rgb(241, 196, 15)
			    ColorType(88, 177, 159, 255),  // rgb(88, 177, 159)
			    ColorType(27, 20, 100, 255),   // rgb(27, 20, 100)
			    ColorType(87, 88, 187, 255),   // rgb(87, 88, 187)
			    ColorType(255, 255, 255, 255)  // rgb(255, 255, 255)
			};

			auto col = cols[index % cols.size()];
			if constexpr (std::is_same<NumberType, int>::value) {
				return col;
			} else {
				return col / 255.0F;
			}
		}

		[[nodiscard]] ColorType lerp(const ColorType& other, float timestep) const {
			if ((*this) == other) return other;
			ColorType ret;

			ret.r = static_cast<NumberType>(static_cast<float>(r) + static_cast<float>(other.r - r) * timestep);
			ret.g = static_cast<NumberType>(static_cast<float>(g) + static_cast<float>(other.g - g) * timestep);
			ret.b = static_cast<NumberType>(static_cast<float>(b) + static_cast<float>(other.b - b) * timestep);
			ret.a = static_cast<NumberType>(static_cast<float>(a) + static_cast<float>(other.a - a) * timestep);

			return ret;
		}

		[[nodiscard]] bool isTransparent() const { return a == 0; }
		[[nodiscard]] NumberType dot(const ColorType& other) const {
			return r * other.r + g * other.g + b * other.b + a * other.a;
		}

		[[nodiscard]] uint32_t toHEX() const {
			if constexpr (std::is_same<NumberType, int>::value) {
				Color_t<float> cc = this->cast<float>();
				return rawrbox::PackUtils::toAbgr(cc.r, cc.g, cc.b, cc.a);
			} else {
				return rawrbox::PackUtils::toAbgr(this->r, this->g, this->b, this->a);
			}
		}

		ColorType operator-(const ColorType& other) const {
			return ColorType(r - other.r, g - other.g, b - other.b, a - other.a);
		}

		ColorType operator+(const ColorType& other) const {
			return ColorType(r + other.r, g + other.g, b + other.b, a + other.a);
		}

		ColorType operator*(const ColorType& other) const {
			return ColorType(r * other.r, g * other.g, b * other.b, a * other.a);
		}

		ColorType operator/(const ColorType& other) const {
			return ColorType(r / other.r, g / other.g, b / other.b, a / other.a);
		}

		ColorType operator-(NumberType other) const {
			return ColorType(r - other, g - other, b - other, a - other);
		}

		ColorType operator+(NumberType other) const {
			return ColorType(r + other, g + other, b + other, a + other);
		}

		ColorType operator*(NumberType scale) const {
			return ColorType(r * scale, g * scale, b * scale, a * scale);
		}

		ColorType operator/(NumberType scale) const {
			return ColorType(r / scale, g / scale, b / scale, a / scale);
		}

		ColorType& operator-=(const ColorType& other) {
			r -= other.r;
			g -= other.g;
			b -= other.b;
			a -= other.a;
			return *this;
		}

		ColorType& operator+=(const ColorType& other) {
			r += other.r;
			g += other.g;
			b += other.b;
			a += other.a;
			return *this;
		}

		ColorType& operator*=(const ColorType& other) {
			r *= other.r;
			g *= other.g;
			b *= other.b;
			a *= other.a;
			return *this;
		}

		ColorType& operator/=(const ColorType& other) {
			r /= other.r;
			g /= other.g;
			b /= other.b;
			a /= other.a;
			return *this;
		}

		ColorType& operator-=(NumberType other) {
			r -= other;
			g -= other;
			b -= other;
			a -= other;
			return *this;
		}

		ColorType& operator+=(NumberType other) {
			r += other;
			g += other;
			b += other;
			a += other;
			return *this;
		}

		ColorType& operator*=(NumberType other) {
			r *= other;
			g *= other;
			b *= other;
			a *= other;
			return *this;
		}

		ColorType& operator/=(NumberType other) {
			r /= other;
			g /= other;
			b /= other;
			a /= other;
			return *this;
		}

		ColorType operator-() const {
			return ColorType(-r, -g, -b, -a);
		}

		bool operator==(const ColorType& other) const {
			return r == other.r && g == other.g && b == other.b && a == other.a;
		}

		bool operator!=(const ColorType& other) const {
			return !operator==(other);
		}
	};

	using Colorf = Color_t<float>;
	using Colori = Color_t<int>;
	using Color = Colorf;

	template <class NumberType>
	class Colors_t {
	private:
		using ColorType = Color_t<NumberType>;

	public:
		inline const static ColorType Black = ColorType(0, 0, 0, 1);
		inline const static ColorType White = ColorType(1, 1, 1, 1);

		inline const static ColorType Gray = ColorType(0.81F, 0.8F, 0.75F, 1);
		inline const static ColorType DarkGray = ColorType(0.51F, 0.50F, 0.47F, 1);

		inline const static ColorType Red = ColorType(1, 0.32F, 0.32F, 1);
		inline const static ColorType Green = ColorType(0.14F, 0.87F, 0.50F, 1);
		inline const static ColorType Blue = ColorType(0.2F, 0.67F, 0.87F, 1);
		inline const static ColorType Orange = ColorType(1, 0.47F, 0.24F, 1);
		inline const static ColorType Yellow = ColorType(1, 0.69F, 0.25F, 1);
		inline const static ColorType Purple = ColorType(1.F, 0.F, 1.F, 1);

		inline const static ColorType Transparent = ColorType(0, 0, 0, 0);
	};

	using Colorsf = Colors_t<float>;
	using Colorsi = Colors_t<int>;
	using Colors = Colorsf;
} // namespace rawrbox
