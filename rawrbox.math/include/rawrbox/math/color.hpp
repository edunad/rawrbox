#pragma once

#include <rawrbox/math/vector3.hpp>

#include <array>
#include <cstdint>

namespace rawrbox {
	template <class NumberType>
		requires(std::is_integral_v<NumberType> || std::is_floating_point_v<NumberType>)
	class Color_t {
	private:
		using ColorType = Color_t<NumberType>;

	public:
		NumberType r = 0, g = 0, b = 0, a = 1;

		Color_t() = default;
		Color_t(NumberType _r, NumberType _g, NumberType _b, NumberType _a) : r(_r), g(_g), b(_b), a(_a) {}

		static Color_t<NumberType> RGBAHex(uint32_t x) {
			if constexpr (std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>) {
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
			if constexpr (std::is_same_v<NumberType, float> || std::is_same_v<NumberType, double>) {
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

		[[nodiscard]] NumberType GammaToLinear(const NumberType& x) const {
			auto val = static_cast<float>(x);
			return val <= 0.04045F ? static_cast<NumberType>(val / 12.92F) : static_cast<NumberType>(std::pow((val + 0.055F) / 1.055F, 2.4F));
		}

		[[nodiscard]] NumberType LinearToGamma(const NumberType& x) const {
			auto val = static_cast<float>(x);
			return val <= 0.0031308 ? static_cast<NumberType>(val * 12.92F) : static_cast<NumberType>(1.055F * std::pow(val, 1.F / 2.4F) - 0.055F);
		}

		[[nodiscard]] Color_t<NumberType> toLinear() const {
			return {GammaToLinear(r), GammaToLinear(g), GammaToLinear(b), a};
		}

		[[nodiscard]] Color_t<NumberType> toSRGB() const {
			return {LinearToGamma(r), LinearToGamma(g), LinearToGamma(b), a};
		}

		[[nodiscard]] std::string toHEX() const {
			std::array<char, 10> hexColor = {};
			std::snprintf(hexColor.data(), sizeof(hexColor), "#%02x%02x%02x%02x", static_cast<int>(this->r), static_cast<int>(this->g), static_cast<int>(this->b), static_cast<int>(this->a));

			return {hexColor.begin(), hexColor.end()};
		}

		template <class ReturnType>
		[[nodiscard]] Color_t<ReturnType> cast() const {
			if constexpr (std::is_same_v<NumberType, ReturnType>) return *this;

			if constexpr (std::is_same_v<NumberType, int>) {
				return Color_t<ReturnType>(
				    static_cast<ReturnType>(r) / 255.0F,
				    static_cast<ReturnType>(g) / 255.0F,
				    static_cast<ReturnType>(b) / 255.0F,
				    static_cast<ReturnType>(a) / 255.0F);
			} else {
				return Color_t<ReturnType>(
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
			    ColorType(212, 24, 108, 255),  // rgb(212, 24, 108)
			    ColorType(255, 0, 255, 255),   // rgb(252, 0, 255)
			    ColorType(241, 196, 15, 255),  // rgb(241, 196, 15)
			    ColorType(88, 177, 159, 255),  // rgb(88, 177, 159)
			    ColorType(27, 20, 100, 255),   // rgb(27, 20, 100)
			    ColorType(87, 88, 187, 255),   // rgb(87, 88, 187)
			    ColorType(255, 255, 255, 255)  // rgb(255, 255, 255)
			};

			auto col = cols[index % cols.size()];
			if constexpr (std::is_same_v<NumberType, int>) {
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

		[[nodiscard]] ColorType strength(float strength) const {
			if constexpr (std::is_same_v<NumberType, int>) {
				return {static_cast<int>(r * strength), static_cast<int>(g * strength), static_cast<int>(b * strength), 255};
			} else {
				return {r * strength, g * strength, b * strength, 1.F};
			}
		}

		[[nodiscard]] const std::array<NumberType, 4> array() const {
			return {r, g, b, a};
		}

		[[nodiscard]] const rawrbox::Vector3_t<NumberType> rgb() const {
			return {r, g, b};
		}

		[[nodiscard]] const rawrbox::Vector3_t<NumberType> bgr() const {
			return {b, g, r};
		}

		[[nodiscard]] const ColorType bgra() const {
			return {b, g, r, a};
		}

		[[nodiscard]] const NumberType max() const {
			return std::max(std::max(r, g), b);
		}

		[[nodiscard]] bool invisible() const { return a == 0; }
		[[nodiscard]] bool hasTransparency() const {
			if constexpr (std::is_same_v<NumberType, int>) {
				return a != 255;
			} else {
				return a != 1.F;
			}
		}

		[[nodiscard]] NumberType dot(const ColorType& other) const {
			return r * other.r + g * other.g + b * other.b + a * other.a;
		}

		[[nodiscard]] uint32_t pack() const {
			return pack(this->r, this->g, this->b, this->a);
		}

		[[nodiscard]] static uint32_t pack(NumberType r, NumberType g, NumberType b, NumberType a) {
			if constexpr (std::is_same_v<NumberType, int>) {
				return 0 | (static_cast<uint8_t>(r) << 0) | (static_cast<uint8_t>(g) << 8) | (static_cast<uint8_t>(b) << 16) | (static_cast<uint8_t>(a) << 24);
			} else {
				return 0 | (static_cast<uint8_t>(r * 255.0F) << 0) | (static_cast<uint8_t>(g * 255.0F) << 8) | (static_cast<uint8_t>(b * 255.0F) << 16) | (static_cast<uint8_t>(a * 255.0F) << 24);
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
	using Colord = Color_t<double>;
	using Color = Colorf;

	template <class NumberType>
	class Colors_t {
	private:
		using ColorType = Color_t<NumberType>;

	public:
		[[nodiscard]] static inline ColorType Black() {
			if constexpr (std::is_same_v<NumberType, int>) {
				return ColorType(0, 0, 0, 255);
			} else {
				return ColorType(0, 0, 0, 1);
			}
		}

		[[nodiscard]] static inline ColorType White() {
			if constexpr (std::is_same_v<NumberType, int>) {
				return ColorType(255, 255, 255, 255);
			} else {
				return ColorType(1, 1, 1, 1);
			}
		}

		[[nodiscard]] static inline ColorType Gray() {
			if constexpr (std::is_same_v<NumberType, int>) {
				return ColorType(206, 204, 191, 255);
			} else {
				return ColorType(0.81F, 0.8F, 0.75F, 1);
			}
		}

		[[nodiscard]] static inline ColorType Red() {
			if constexpr (std::is_same_v<NumberType, int>) {
				return ColorType(255, 82, 82, 255);
			} else {
				return ColorType(1, 0.32F, 0.32F, 1);
			}
		}

		[[nodiscard]] static inline ColorType Green() {
			if constexpr (std::is_same_v<NumberType, int>) {
				return ColorType(36, 222, 128, 255);
			} else {
				return ColorType(0.14F, 0.87F, 0.50F, 1);
			}
		}

		[[nodiscard]] static inline ColorType Blue() {
			if constexpr (std::is_same_v<NumberType, int>) {
				return ColorType(51, 171, 222, 255);
			} else {
				return ColorType(0.2F, 0.67F, 0.87F, 1);
			}
		}

		[[nodiscard]] static inline ColorType Orange() {
			if constexpr (std::is_same_v<NumberType, int>) {
				return ColorType(255, 120, 61, 255);
			} else {
				return ColorType(1, 0.47F, 0.24F, 1);
			}
		}

		[[nodiscard]] static inline ColorType Yellow() {
			if constexpr (std::is_same_v<NumberType, int>) {
				return ColorType(255, 230, 64, 255);
			} else {
				return ColorType(1, 0.9F, 0.25F, 1);
			}
		}

		[[nodiscard]] static inline ColorType Purple() {
			if constexpr (std::is_same_v<NumberType, int>) {
				return ColorType(255, 0, 255, 255);
			} else {
				return ColorType(1.F, 0.F, 1.F, 1);
			}
		}

		[[nodiscard]] static inline ColorType Brown() {
			if constexpr (std::is_same_v<NumberType, int>) {
				return ColorType(133, 88, 49, 255);
			} else {
				return ColorType(0.52F, 0.35F, 0.19F, 1);
			}
		}

		[[nodiscard]] static inline ColorType Transparent() {
			return ColorType(0, 0, 0, 0);
		}
	};

	using Colorsf = Colors_t<float>;
	using Colorsi = Colors_t<int>;
	using Colorsd = Colors_t<double>;
	using Colors = Colorsf;
} // namespace rawrbox
